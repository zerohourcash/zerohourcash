/*
    This file is part of cpp-ethereum.

    cpp-ethereum is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cpp-ethereum is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <thread>
#include <list>
#include <map>
#include <mutex>
#include <libdevcore/db.h>
#include <libdevcore/Common.h>
#include <util/system.h>
#include "SHA3.h"
#include "OverlayDB.h"
#include "TrieDB.h"

namespace dev
{
namespace
{
inline db::Slice toSlice(h256 const& _h)
{
    return db::Slice(reinterpret_cast<char const*>(_h.data()), _h.size);
}

inline db::Slice toSlice(std::string const& _str)
{
    return db::Slice(_str.data(), _str.size());
}

inline db::Slice toSlice(bytes const& _b)
{
    return db::Slice(reinterpret_cast<char const*>(&_b[0]), _b.size());
}

size_t configuredLookupCacheBytes()
{
    int64_t const value = gArgs.GetArg("-zhcstatelookupcache", 256);
    if (value <= 0)
        return 0;
    return static_cast<size_t>(value) << 20;
}

class DiskLookupCache
{
public:
    bool get(h256 const& _hash, std::string& _value)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        resizeLimit();
        if (m_limitBytes == 0)
            return false;
        auto it = m_index.find(_hash);
        if (it == m_index.end())
            return false;
        m_lru.splice(m_lru.begin(), m_lru, it->second);
        _value = it->second->second;
        return true;
    }

    void put(h256 const& _hash, std::string const& _value)
    {
        if (_value.empty())
            return;
        std::lock_guard<std::mutex> lock(m_mutex);
        resizeLimit();
        if (m_limitBytes == 0)
            return;
        size_t const entryBytes = estimateEntryBytes(_value);
        if (entryBytes > m_limitBytes)
            return;

        auto it = m_index.find(_hash);
        if (it != m_index.end())
        {
            m_bytes -= estimateEntryBytes(it->second->second);
            m_lru.erase(it->second);
            m_index.erase(it);
        }

        m_lru.push_front(std::make_pair(_hash, _value));
        m_index[_hash] = m_lru.begin();
        m_bytes += entryBytes;
        trim();
    }

    void erase(h256 const& _hash)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_index.find(_hash);
        if (it == m_index.end())
            return;
        m_bytes -= estimateEntryBytes(it->second->second);
        m_lru.erase(it->second);
        m_index.erase(it);
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_lru.clear();
        m_index.clear();
        m_bytes = 0;
        m_limitBytes = configuredLookupCacheBytes();
    }

private:
    using LruEntry = std::pair<h256, std::string>;
    using LruList = std::list<LruEntry>;

    static size_t estimateEntryBytes(std::string const& _value)
    {
        return sizeof(h256) + sizeof(LruEntry) + _value.size();
    }

    void resizeLimit()
    {
        size_t const newLimit = configuredLookupCacheBytes();
        if (newLimit == m_limitBytes)
            return;
        m_limitBytes = newLimit;
        trim();
    }

    void trim()
    {
        while (m_bytes > m_limitBytes && !m_lru.empty())
        {
            auto const& entry = m_lru.back();
            m_bytes -= estimateEntryBytes(entry.second);
            m_index.erase(entry.first);
            m_lru.pop_back();
        }
    }

    std::mutex m_mutex;
    size_t m_limitBytes = 0;
    size_t m_bytes = 0;
    LruList m_lru;
    std::map<h256, LruList::iterator> m_index;
};

DiskLookupCache& diskLookupCache()
{
    static DiskLookupCache cache;
    return cache;
}

}  // namespace

OverlayDB::~OverlayDB() = default;

void OverlayDB::commit()
{
    if (m_db)
    {
        auto writeBatch = m_db->createWriteBatch();
//      cnote << "Committing nodes to disk DB:";
#if DEV_GUARDED_DB
        DEV_READ_GUARDED(x_this)
#endif
        {
            for (auto const& i: m_main)
            {
                if (i.second.second)
                    writeBatch->insert(toSlice(i.first), toSlice(i.second.first));
//              cnote << i.first << "#" << m_main[i.first].second;
            }
            for (auto const& i: m_aux)
                if (i.second.second)
                {
                    bytes b = i.first.asBytes();
                    b.push_back(255);   // for aux
                    writeBatch->insert(toSlice(b), toSlice(i.second.first));
                }
        }

        for (unsigned i = 0; i < 10; ++i)
        {
            try
            {
                m_db->commit(std::move(writeBatch));
                break;
            }
            catch (boost::exception const& ex)
            {
                if (i == 9)
                {
                    cwarn << "Fail writing to state database. Bombing out.";
                    exit(-1);
                }
                cwarn << "Error writing to state database: " << boost::diagnostic_information(ex);
                cwarn << "Sleeping for" << (i + 1) << "seconds, then retrying.";
                std::this_thread::sleep_for(std::chrono::seconds(i + 1));
            }
        }
#if DEV_GUARDED_DB
        DEV_WRITE_GUARDED(x_this)
#endif
        {
            m_aux.clear();
            m_main.clear();
        }
    }
}

bytes OverlayDB::lookupAux(h256 const& _h) const
{
    bytes ret = StateCacheDB::lookupAux(_h);
    if (!ret.empty() || !m_db)
        return ret;

    bytes b = _h.asBytes();
    b.push_back(255);   // for aux
    std::string const v = m_db->lookup(toSlice(b));
    if (v.empty())
        cwarn << "Aux not found: " << _h;

    return asBytes(v);
}

void OverlayDB::rollback()
{
#if DEV_GUARDED_DB
    WriteGuard l(x_this);
#endif
    m_main.clear();
}

std::string OverlayDB::lookup(h256 const& _h) const
{
    std::string ret = StateCacheDB::lookup(_h);
    if (!ret.empty() || !m_db)
        return ret;

    if (diskLookupCache().get(_h, ret))
        return ret;

    ret = m_db->lookup(toSlice(_h));
    diskLookupCache().put(_h, ret);
    return ret;
}

bool OverlayDB::exists(h256 const& _h) const
{
    if (StateCacheDB::exists(_h))
        return true;
    return m_db && m_db->exists(toSlice(_h));
}

void OverlayDB::kill(h256 const& _h)
{
    diskLookupCache().erase(_h);
#if ETH_PARANOIA || 1
    if (!StateCacheDB::kill(_h))
    {
        if (m_db)
        {
            if (!m_db->exists(toSlice(_h)))
            {
                // No point node ref decreasing for EmptyTrie since we never bother incrementing it
                // in the first place for empty storage tries.
                if (_h != EmptyTrie)
                    cnote << "Decreasing DB node ref count below zero with no DB node. Probably "
                             "have a corrupt Trie."
                          << _h;
                // TODO: for 1.1: ref-counted triedb.
            }
        }
    }
#else
    StateCacheDB::kill(_h);
#endif
}

void OverlayDB::clearLookupCacheForTesting()
{
    diskLookupCache().clear();
}

}
