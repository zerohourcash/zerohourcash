#ifndef TOKEN_H
#define TOKEN_H
#include <string>
#include <vector>
#include <uint256.h>

#include <QString>

struct TokenEvent{
    std::string address;
    std::string sender;
    std::string receiver;
    uint256 blockHash;
    uint64_t blockNumber;
    uint256 transactionHash;
    uint256 value;
    std::string contractType;

    TokenEvent()
    {
        SetNull();
    }

    void SetNull()
    {
        blockHash.SetNull();
        blockNumber = 0;
        transactionHash.SetNull();
        value.SetNull();
    }
};

struct TokenData;
class WalletModel;

class Token
{
public:
    Token();
    ~Token();

    void setModel(WalletModel *model);

    // Set command data
    void setAddress(const std::string &address);
    void setDataHex(const std::string &datahex);
    void setAmount(const std::string &amount);
    void setGasLimit(const std::string &gaslimit);
    void setGasPrice(const std::string &gasPrice);
    void setSender(const std::string &sender);
    void setReceiver(const std::string &reseiver);
    void clear();

    // Get transaction data
    std::string getTxId();
    std::string getErrorMessage();

    // ABI Functions
    bool name(std::string& result, bool sendTo = false);
    bool approve(const std::string& _spender, const std::string& _value, bool& success, bool sendTo = false);
    bool totalSupply(std::string& result, bool sendTo = false);
    bool transferFrom(const std::string& _from, const std::string& _to, const std::string& _value, bool& success, bool sendTo = false);
    bool decimals(std::string& result, bool sendTo = false);
    bool burn(const std::string& _value, bool& success, bool sendTo = false);
    bool balanceOf(std::string& result, bool sendTo = false);
    bool balanceOf(const std::string& spender, std::string& result, bool sendTo = false);
    bool tokenURI(std::string &result, QString &jsonResult, std::string tokenId = "1");
    bool burnFrom(const std::string& _from, const std::string& _value, bool& success, bool sendTo = false);
    bool symbol(std::string& result, bool sendTo = false);
    bool transfer(const std::string& _to, const std::string& _value, bool sendTo = false);
    bool approveAndCall(const std::string& _spender, const std::string& _value, const std::string& _extraData, bool& success, bool sendTo = false);
    bool allowance(const std::string& _from, const std::string& _to, std::string& result, bool sendTo = false);

    // ABI Events
    bool transferEvents(std::vector<TokenEvent>& tokenEvents, int64_t fromBlock = 0, int64_t toBlock = -1);
    bool burnEvents(std::vector<TokenEvent>& tokenEvents, int64_t fromBlock = 0, int64_t toBlock = -1);

    int errorNumber = 1;

    QString resultJsonOut;

private:
    bool exec(const std::vector<std::string>& input, int func, std::vector<std::string>& output, bool sendTo);
    bool execEvents(int64_t fromBlock, int64_t toBlock, int func, std::vector<TokenEvent> &tokenEvents);

    Token(Token const&);
    Token& operator=(Token const&);

private:
    TokenData* d;
};

#endif // TOKEN_H
