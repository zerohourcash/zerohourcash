#ifndef ZHC_QT_TOKENTRANSACTIONRECORD_H
#define ZHC_QT_TOKENTRANSACTIONRECORD_H

#include <amount.h>
#include <uint256.h>
#include <util/convert.h>

#include <QList>
#include <QString>

namespace interfaces {
class Wallet;
struct TokenTx;
}
/** UI model for token transaction status. The token transaction status is the part of a token transaction that will change over time.
 */
class TokenTransactionStatus
{
public:
    TokenTransactionStatus():
        countsForBalance(false), sortKey(""),
        status(Unconfirmed), depth(0), cur_num_blocks(-1)
    { }

    enum Status {
        Confirmed,          /**< Have 6 or more confirmations (normal tx) or fully mature (mined tx) **/
        /// Normal (sent/received) token transactions
        Unconfirmed,        /**< Not yet mined into a block **/
        Confirming         /**< Confirmed, but waiting for the recommended number of confirmations **/
    };

    /// Token transaction counts towards available balance
    bool countsForBalance;
    /// Sorting key based on status
    std::string sortKey;

    /** @name Reported status
       @{*/
    Status status;
    qint64 depth;

    /**@}*/

    /** Current number of blocks (to know whether cached status is still valid) */
    int cur_num_blocks;
};

/** UI model for a token transaction. A core token transaction can be represented by multiple UI token transactions if it has
    multiple outputs.
 */
class TokenTransactionRecord
{
public:
    enum Type
    {
        Other,
        SendToAddress,
        SendToOther,
        RecvWithAddress,
        RecvFromOther,
        SendToSelf
    };

    /** Number of confirmation recommended for accepting a token transaction */
    static const int RecommendedNumConfirmations = 10;

    TokenTransactionRecord():
            hash(), txid(), time(0), type(Other), address(""), debit(0), credit(0), label("")
    {
    }

    /** Decompose Token transaction into a record.
     */
    static QList<TokenTransactionRecord> decomposeTransaction(interfaces::Wallet &wallet, const interfaces::TokenTx &wtx);

    /** @name Immutable token transaction attributes
      @{*/
    uint256 hash;
    uint256 txid;
    qint64 time;
    Type type;
    std::string address;
    dev::s256 debit;
    dev::s256 credit;
    std::string tokenSymbol;
    uint8_t decimals;
    std::string label;
    std::string contractType = "-1";
    std::string contractAddress;
    std::string tokenURI;

    std::string senderWallet;
    std::string receiverWallet;

    /**@}*/

    /** Return the unique identifier for this transaction (part) */
    QString getTxID() const;

    /** Status: can change with block chain update */
    TokenTransactionStatus status;

    /** Update status from core wallet tx.
     */
    void updateStatus(int block_number, int num_blocks);

    /** Return whether a status update is needed.
     */
    bool statusUpdateNeeded(int numBlocks);
};

#endif // BITCOIN_QT_TRANSACTIONRECORD_H
