#include <qt/sendtokenpage.h>
#include <qt/forms/ui_sendtokenpage.h>

#include <qt/walletmodel.h>
#include <qt/clientmodel.h>
#include <qt/optionsmodel.h>
#include <validation.h>
#include <util/moneystr.h>
#include <util/convert.h>
#include <qt/token.h>
#include <qt/bitcoinunits.h>
#include <wallet/wallet.h>
#include <validation.h>
#include <qt/guiutil.h>
#include <qt/sendcoinsdialog.h>
#include <qt/bitcoinaddressvalidator.h>
#include <uint256.h>
#include <qt/styleSheet.h>
#include <interfaces/node.h>

#include <QDebug>

static const CAmount SINGLE_STEP = 0.00000001*COIN;

struct SelectedToken{
    std::string address;
    std::string sender;
    std::string symbol;
    int type;
    int8_t decimals;
    std::string balance;
    SelectedToken():
        decimals(0)
    {}
};

SendTokenPage::SendTokenPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SendTokenPage),
    m_model(0),
    m_clientModel(0),
    m_tokenABI(0),
    m_selectedToken(0)
{
    // Setup ui components
    ui->setupUi(this);

    // Set stylesheet
    SetObjectStyleSheet(ui->clearButton, StyleSheetNames::ButtonBlack);
    ui->labelPayTo->setToolTip(tr("The address that will receive the tokens."));
    ui->labelAmount->setToolTip(tr("The amount in Token to send."));
    ui->labelDescription->setToolTip(tr("Optional description for transaction."));
    m_tokenABI = new Token();
    m_selectedToken = new SelectedToken();
    // Set defaults
    ui->lineEditGasPrice->setValue(DEFAULT_GAS_PRICE);
    ui->lineEditGasPrice->setSingleStep(SINGLE_STEP);
    ui->lineEditGasLimit->setMaximum(DEFAULT_GAS_LIMIT_OP_SEND);
    ui->lineEditGasLimit->setValue(DEFAULT_GAS_LIMIT_OP_SEND);
    ui->confirmButton->setEnabled(false);

    // Connect signals with slots
    connect(ui->lineEditPayTo, &QValidatedLineEdit::textChanged, this, &SendTokenPage::on_updateConfirmButton);
    connect(ui->lineEditAmount, &TokenAmountField::valueChanged,this, &SendTokenPage::on_updateConfirmButton);
    connect(ui->confirmButton, &QPushButton::clicked, this, &SendTokenPage::on_confirmClicked);

    ui->lineEditPayTo->setCheckValidator(new BitcoinAddressCheckValidator(parent, true));

}

SendTokenPage::~SendTokenPage()
{
    delete ui;

    if(m_tokenABI)
        delete m_tokenABI;
    m_tokenABI = 0;
}

void SendTokenPage::setNftSelect(QString contractAddress, QString senderAddress)
{

    ui->selectNft->clear();

    QList<TokenTransactionRecord> t_records;
    for(interfaces::TokenTx wtokenTx : m_model->wallet().getTokenTxs())
    {
        t_records.append(TokenTransactionRecord::decomposeTransaction(m_model->wallet(), wtokenTx));
    }

    if(!t_records.isEmpty())
	{

	QList<int> *intResult = new QList<int>();
	QList<TokenTransactionRecord> *listResult = new QList<TokenTransactionRecord>();

	for(int i = 0; i < t_records.size(); i++)
	{
    	    TokenTransactionRecord record = t_records.at(i);

    	    if(record.contractType != "1")
    	    {
		continue;
    	    }

	    if(QString::fromStdString(record.contractAddress) == contractAddress)
	    {

    		QString tempIdCredit = BitcoinUnits::formatTokenWithUnit("", record.decimals, record.credit, false);
    		QString tempIdDebit = BitcoinUnits::formatTokenWithUnit("", record.decimals, record.debit, false);

    		int temp = 0;
		int tempCredit = tempIdCredit.toInt();
		int tempDebit = tempIdDebit.toInt();

		if(QString::fromStdString(record.receiverWallet) == senderAddress)
		{
    		    if(tempCredit > 0 && tempDebit == 0)
    		    {
			temp = tempCredit;
    		    }
		}
        
		if(QString::fromStdString(record.senderWallet) == senderAddress)
		{
    		    if(tempCredit == 0 && tempDebit < 0)
    		    {
			temp = tempDebit;
    		    }
		}

		if(temp > 0)
		{
    		    listResult->push_back(record);
		    intResult->push_back(temp);
		}
		else
		{
    		    int del = intResult->indexOf(abs(temp));
    		    if(del != -1)
    		    {
        		listResult->removeAt(del);
        		intResult->removeAt(del);
    		    }
		}
	    }
	}

	for(int i = 0; i < listResult->size(); i++)
	{
	    TokenTransactionRecord record = listResult->at(i);
	    QString tokenId = BitcoinUnits::formatTokenWithUnit("", record.decimals, record.credit, false);
	    ui->selectNft->addItem(QString::fromStdString(record.tokenSymbol) + " ID:" + tokenId, tokenId.toInt());
	}


	if(!listResult->isEmpty())
	{
	    TokenTransactionRecord lastRecord = listResult->first();
	    QString tokenIdLast = BitcoinUnits::formatTokenWithUnit("", lastRecord.decimals, lastRecord.credit, false);
	    ui->lineEditAmount->setValue(tokenIdLast.toInt());
	}
	else
	{
	    ui->selectNft->clear();
	}

    }

}

void SendTokenPage::on_selectNft_currentIndexChanged(int index)
{
    int currentValue = ui->selectNft->currentData().toInt();
    ui->lineEditAmount->setValue(currentValue);
}

void SendTokenPage::setModel(WalletModel *_model)
{
    m_model = _model;
    m_tokenABI->setModel(m_model);

    if (m_model && m_model->getOptionsModel())
        connect(m_model->getOptionsModel(), &OptionsModel::displayUnitChanged, this, &SendTokenPage::updateDisplayUnit);

    // update the display unit, to not use the default ("ZHC")
    updateDisplayUnit();
}

void SendTokenPage::setClientModel(ClientModel *_clientModel)
{
    m_clientModel = _clientModel;

    if (m_clientModel)
    {
        connect(m_clientModel, SIGNAL(gasInfoChanged(quint64, quint64, quint64)), this, SLOT(on_gasInfoChanged(quint64, quint64, quint64)));
    }
}

void SendTokenPage::clearAll()
{
    ui->lineEditPayTo->setText("");
    ui->lineEditAmount->clear();
    ui->lineEditDescription->setText("");
    ui->lineEditGasLimit->setValue(DEFAULT_GAS_LIMIT_OP_SEND);
    ui->lineEditGasPrice->setValue(DEFAULT_GAS_PRICE);
}

bool SendTokenPage::isValidAddress()
{
    ui->lineEditPayTo->checkValidity();
    return ui->lineEditPayTo->isValid();
}

bool SendTokenPage::isDataValid()
{
    bool dataValid = true;

    if(!isValidAddress())
        dataValid = false;
    if(!ui->lineEditAmount->validate())
        dataValid = false;
    if(ui->lineEditAmount->value(0) <= 0)
    {
        ui->lineEditAmount->setValid(false);
        dataValid = false;
    }
    return dataValid;
}

void SendTokenPage::on_clearButton_clicked()
{
    clearAll();
}

void SendTokenPage::on_gasInfoChanged(quint64 blockGasLimit, quint64 minGasPrice, quint64 nGasPrice)
{
    Q_UNUSED(nGasPrice);
    ui->labelGasLimit->setToolTip(tr("Gas limit. Default = %1, Max = %2").arg(DEFAULT_GAS_LIMIT_OP_CREATE).arg(blockGasLimit));
    ui->labelGasPrice->setToolTip(tr("Gas price: ZHC price per gas unit. Default = %1, Min = %2").arg(QString::fromStdString(FormatMoney(DEFAULT_GAS_PRICE))).arg(QString::fromStdString(FormatMoney(minGasPrice))));
    ui->lineEditGasPrice->SetMinValue(minGasPrice);
    ui->lineEditGasLimit->setMaximum(blockGasLimit);
}

void SendTokenPage::on_updateConfirmButton()
{
    bool enabled = true;
    if(ui->lineEditPayTo->text().isEmpty() || ui->lineEditAmount->text().isEmpty())
    {
        enabled = false;
    }

    ui->confirmButton->setEnabled(enabled);
}

void SendTokenPage::on_confirmClicked()
{
    if(!isDataValid())
        return;

    WalletModel::UnlockContext ctx(m_model->requestUnlock());
    if(!ctx.isValid())
    {
        return;
    }

    if(m_model)
    {
        int unit = BitcoinUnits::BTC;
        uint64_t gasLimit = ui->lineEditGasLimit->value();
        CAmount gasPrice = ui->lineEditGasPrice->value();
        std::string label = ui->lineEditDescription->text().trimmed().toStdString();

        m_tokenABI->setAddress(m_selectedToken->address);
        m_tokenABI->setSender(m_selectedToken->sender);
        m_tokenABI->setGasLimit(QString::number(gasLimit).toStdString());
        m_tokenABI->setGasPrice(BitcoinUnits::format(unit, gasPrice, false, BitcoinUnits::separatorNever).toStdString());

        std::string toAddress = ui->lineEditPayTo->text().toStdString();
        std::string amountToSend = ui->lineEditAmount->text().toStdString();
        QString amountFormated = BitcoinUnits::formatToken(m_selectedToken->decimals, ui->lineEditAmount->value(), false, BitcoinUnits::separatorAlways);

        QString questionString = tr("Are you sure you want to send? <br /><br />");
        questionString.append(tr("<b>%1 %2 </b> to ")
                              .arg(amountFormated).arg(QString::fromStdString(m_selectedToken->symbol)));
        questionString.append(tr("<br />%3 <br />")
                              .arg(QString::fromStdString(toAddress)));

        SendConfirmationDialog confirmationDialog(tr("Confirm send token."), questionString, 3, this);
        confirmationDialog.exec();
        QMessageBox::StandardButton retval = (QMessageBox::StandardButton)confirmationDialog.result();
        if(retval == QMessageBox::Yes)
        {
            if(m_tokenABI->transfer(toAddress, amountToSend, true))
            {
                interfaces::TokenTx tokenTx;
                tokenTx.contract_address = m_selectedToken->address;
                tokenTx.sender_address = m_selectedToken->sender;
                tokenTx.receiver_address = toAddress;
                dev::u256 nValue(amountToSend);
                tokenTx.value = u256Touint(nValue);
                tokenTx.tx_hash = uint256S(m_tokenABI->getTxId());
                tokenTx.label = label;
                m_model->wallet().addTokenTxEntry(tokenTx);
            }
            else
            {
                QMessageBox::warning(this, tr("Send token"), QString::fromStdString(m_tokenABI->getErrorMessage()));
            }
            clearAll();
        }
    }
}

void SendTokenPage::updateDisplayUnit()
{
    if(m_model && m_model->getOptionsModel())
    {
        // Update gasPriceAmount with the current unit
        ui->lineEditGasPrice->setDisplayUnit(m_model->getOptionsModel()->getDisplayUnit());
    }
}

void SendTokenPage::setTokenData(std::string address, std::string sender, std::string symbol, int8_t decimals, std::string balance, int type)
{
    // Update data with the current token
    int decimalDiff = decimals - m_selectedToken->decimals;
    m_selectedToken->address = address;
    m_selectedToken->sender = sender;
    m_selectedToken->symbol = symbol;
    m_selectedToken->decimals = decimals;
    m_selectedToken->balance = balance;
    m_selectedToken->type = type;

    if(m_selectedToken->type == 0)
    {
	ui->labelAmount->setVisible(true);
	ui->lineEditAmount->setVisible(true);

	ui->labelNft->setVisible(false);
	ui->selectNft->setVisible(false);
    }
    else
    {
	setNftSelect(QString::fromStdString(m_selectedToken->address), QString::fromStdString(m_selectedToken->sender));

	ui->labelAmount->setVisible(false);
	ui->lineEditAmount->setVisible(false);

	ui->labelNft->setVisible(true);
	ui->selectNft->setVisible(true);
    }

    // Convert values for different number of decimals
    int256_t totalSupply(balance);
    int tokenType = m_selectedToken->type;
    int256_t value(ui->lineEditAmount->value());
    if(value != 0)
    {
        for(int i = 0; i < decimalDiff; i++)
        {
            value *= 10;
        }
        for(int i = decimalDiff; i < 0; i++)
        {
            value /= 10;
        }
    }

    if(value > totalSupply)
    {
        value = totalSupply;
    }

    // Update the amount field with the current token data
    ui->lineEditAmount->clear();
    ui->lineEditAmount->setDecimalUnits(decimals);
    ui->lineEditAmount->setTotalSupply(totalSupply);
    ui->lineEditAmount->setType(tokenType);

    if(value != 0)
    {
        ui->lineEditAmount->setValue(value);
    }
}
