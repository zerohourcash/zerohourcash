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

#include "LevelDB.h"
#include "Assertions.h"

#include <leveldb/cache.h>
#include <leveldb/filter_policy.h>
#include <logging.h>
#include <mutex>
#include <util/system.h>

namespace dev
{
namespace db
{
namespace
{
inline leveldb::Slice toLDBSlice(Slice _slice)
{
    return leveldb::Slice(_slice.data(), _slice.size());
}

DatabaseStatus toDatabaseStatus(leveldb::Status const& _status)
{
    if (_status.ok())
        return DatabaseStatus::Ok;
    else if (_status.IsIOError())
        return DatabaseStatus::IOError;
    else if (_status.IsCorruption())
        return DatabaseStatus::Corruption;
    else if (_status.IsNotFound())
        return DatabaseStatus::NotFound;
    else
        return DatabaseStatus::Unknown;
}

void checkStatus(leveldb::Status const& _status, boost::filesystem::path const& _path = {})
{
    if (_status.ok())
        return;

    DatabaseError ex;
    ex << errinfo_dbStatusCode(toDatabaseStatus(_status))
       << errinfo_dbStatusString(_status.ToString());
    if (!_path.empty())
        ex << errinfo_path(_path.string());

    BOOST_THROW_EXCEPTION(ex);
}

size_t mibArg(char const* _name, int64_t _defaultMiB)
{
    int64_t const value = gArgs.GetArg(_name, _defaultMiB);
    if (value <= 0)
        return 0;
    return static_cast<size_t>(value) << 20;
}

int intArg(char const* _name, int64_t _defaultValue, int _minimum)
{
    int64_t const value = gArgs.GetArg(_name, _defaultValue);
    return static_cast<int>(std::max<int64_t>(value, _minimum));
}

struct LevelDBRuntimeObjects
{
    std::mutex mutex;
    size_t cacheBytes = 0;
    int bloomBits = 0;
    std::unique_ptr<leveldb::Cache> blockCache;
    std::unique_ptr<const leveldb::FilterPolicy> filterPolicy;
};

LevelDBRuntimeObjects& runtimeObjects()
{
    static LevelDBRuntimeObjects objects;
    return objects;
}

class LevelDBWriteBatch : public WriteBatchFace
{
public:
    void insert(Slice _key, Slice _value) override;
    void kill(Slice _key) override;

    leveldb::WriteBatch const& writeBatch() const { return m_writeBatch; }
    leveldb::WriteBatch& writeBatch() { return m_writeBatch; }

private:
    leveldb::WriteBatch m_writeBatch;
};

void LevelDBWriteBatch::insert(Slice _key, Slice _value)
{
    m_writeBatch.Put(toLDBSlice(_key), toLDBSlice(_value));
}

void LevelDBWriteBatch::kill(Slice _key)
{
    m_writeBatch.Delete(toLDBSlice(_key));
}

}  // namespace

leveldb::ReadOptions LevelDB::defaultReadOptions()
{
    return leveldb::ReadOptions();
}

leveldb::WriteOptions LevelDB::defaultWriteOptions()
{
    return leveldb::WriteOptions();
}

leveldb::Options LevelDB::defaultDBOptions()
{
    ConfiguredOptions const configured = configuredOptions();
    leveldb::Options options;
    options.create_if_missing = true;
    options.max_open_files = configured.maxOpenFiles;
    options.write_buffer_size = configured.writeBufferBytes;

    LevelDBRuntimeObjects& objects = runtimeObjects();
    std::lock_guard<std::mutex> lock(objects.mutex);
    if (configured.blockCacheBytes > 0)
    {
        if (!objects.blockCache || objects.cacheBytes != configured.blockCacheBytes)
        {
            objects.blockCache.reset(leveldb::NewLRUCache(configured.blockCacheBytes));
            objects.cacheBytes = configured.blockCacheBytes;
        }
        options.block_cache = objects.blockCache.get();
    }
    if (configured.bloomBitsPerKey > 0)
    {
        if (!objects.filterPolicy || objects.bloomBits != configured.bloomBitsPerKey)
        {
            objects.filterPolicy.reset(leveldb::NewBloomFilterPolicy(configured.bloomBitsPerKey));
            objects.bloomBits = configured.bloomBitsPerKey;
        }
        options.filter_policy = objects.filterPolicy.get();
    }
    return options;
}

LevelDB::ConfiguredOptions LevelDB::configuredOptions()
{
    ConfiguredOptions options;
    options.blockCacheBytes = mibArg("-zhcstatecache", 256);
    options.writeBufferBytes = mibArg("-zhcstatewritebuffer", 64);
    options.maxOpenFiles = intArg("-zhcstatemaxopenfiles", 1024, 64);
    options.bloomBitsPerKey = intArg("-zhcstatebloom", 10, 0);
    return options;
}

LevelDB::LevelDB(boost::filesystem::path const& _path, leveldb::ReadOptions _readOptions,
    leveldb::WriteOptions _writeOptions, leveldb::Options _dbOptions)
  : m_db(nullptr), m_readOptions(std::move(_readOptions)), m_writeOptions(std::move(_writeOptions))
{
    auto db = static_cast<leveldb::DB*>(nullptr);
    auto const status = leveldb::DB::Open(_dbOptions, _path.string(), &db);
    checkStatus(status, _path);

    assert(db);
    m_db.reset(db);

    if (gArgs.GetBoolArg("-zhcstateforcecompact", false)) {
        LogPrintf("Starting EVM state database compaction of %s\n", _path.string());
        m_db->CompactRange(nullptr, nullptr);
        LogPrintf("Finished EVM state database compaction of %s\n", _path.string());
    }
}

std::string LevelDB::lookup(Slice _key) const
{
    leveldb::Slice const key(_key.data(), _key.size());
    std::string value;
    auto const status = m_db->Get(m_readOptions, key, &value);
    if (status.IsNotFound())
        return std::string();

    checkStatus(status);
    return value;
}

bool LevelDB::exists(Slice _key) const
{
    std::string value;
    leveldb::Slice const key(_key.data(), _key.size());
    auto const status = m_db->Get(m_readOptions, key, &value);
    if (status.IsNotFound())
        return false;

    checkStatus(status);
    return true;
}

void LevelDB::insert(Slice _key, Slice _value)
{
    leveldb::Slice const key(_key.data(), _key.size());
    leveldb::Slice const value(_value.data(), _value.size());
    auto const status = m_db->Put(m_writeOptions, key, value);
    checkStatus(status);
}

void LevelDB::kill(Slice _key)
{
    leveldb::Slice const key(_key.data(), _key.size());
    auto const status = m_db->Delete(m_writeOptions, key);
    checkStatus(status);
}

std::unique_ptr<WriteBatchFace> LevelDB::createWriteBatch() const
{
    return std::unique_ptr<WriteBatchFace>(new LevelDBWriteBatch());
}

void LevelDB::commit(std::unique_ptr<WriteBatchFace> _batch)
{
    if (!_batch)
    {
        BOOST_THROW_EXCEPTION(DatabaseError() << errinfo_comment("Cannot commit null batch"));
    }
    auto* batchPtr = dynamic_cast<LevelDBWriteBatch*>(_batch.get());
    if (!batchPtr)
    {
        BOOST_THROW_EXCEPTION(
            DatabaseError() << errinfo_comment("Invalid batch type passed to LevelDB::commit"));
    }
    auto const status = m_db->Write(m_writeOptions, &batchPtr->writeBatch());
    checkStatus(status);
}

void LevelDB::forEach(std::function<bool(Slice, Slice)> _f) const
{
    std::unique_ptr<leveldb::Iterator> itr(m_db->NewIterator(m_readOptions));
    if (itr == nullptr)
    {
        BOOST_THROW_EXCEPTION(DatabaseError() << errinfo_comment("null iterator"));
    }
    auto keepIterating = true;
    for (itr->SeekToFirst(); keepIterating && itr->Valid(); itr->Next())
    {
        auto const dbKey = itr->key();
        auto const dbValue = itr->value();
        Slice const key(dbKey.data(), dbKey.size());
        Slice const value(dbValue.data(), dbValue.size());
        keepIterating = _f(key, value);
    }
}

}  // namespace db
}  // namespace dev
