#include <qt/zhrctoken.h>
#include <qt/forms/ui_zhrctoken.h>
#include <qt/tokenitemmodel.h>
#include <qt/walletmodel.h>
#include <qt/tokentransactionview.h>
#include <qt/tokentransactionrecord.h>
#include <qt/platformstyle.h>
#include <qt/styleSheet.h>
#include <qt/tokenlistshowdialog.h>

#include <QPainter>
#include <QAbstractItemDelegate>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QSizePolicy>
#include <QMenu>
#include <QDebug>

#define TOKEN_SIZE 54
#define SYMBOL_WIDTH 220
#define MARGIN 5

class TokenViewDelegate : public QAbstractItemDelegate
{
public:

    TokenViewDelegate(const PlatformStyle *_platformStyle, QObject *parent) :
        QAbstractItemDelegate(parent),
        platformStyle(_platformStyle)
    {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const
    {
        painter->save();

        QString tokenSymbol = index.data(TokenItemModel::SymbolRole).toString();
        QString tokenBalance = index.data(TokenItemModel::BalanceRole).toString();
	QString receiveAddress = index.data(TokenItemModel::SenderRole).toString();
        QString tokenType = index.data(TokenItemModel::TypeRole).toString();
        QString tokenTypeError = index.data(TokenItemModel::TypeRoleError).toString();

	QString labelTypeText;
	if(tokenTypeError == "1")
	{
	    labelTypeText = " ( loading... )";
	}
	else
	{
	    if(tokenType == "")
	    {
		labelTypeText = " ( ZRC20 )";
	    }
	    else
	    {
		labelTypeText = " ( ZRC721 )";
	    }
	}
	
        QRect mainRect = option.rect;

        bool selected = option.state & QStyle::State_Selected;
        if(selected)
        {
            painter->fillRect(mainRect,QColor("#009ee5"));
        }
        else
        {
            painter->fillRect(mainRect,QColor("#383938"));
        }

        QRect hLineRect(mainRect.left(), mainRect.bottom(), mainRect.width(), 1);
        painter->fillRect(hLineRect, QColor("#2e2e2e"));

        QColor foreground("#dddddd");
        painter->setPen(foreground);

        QFont font = option.font;
        font.setPointSizeF(option.font.pointSizeF() * 1.1);
        font.setBold(true);
        painter->setFont(font);
        QColor amountColor("#ffffff");
        painter->setPen(amountColor);

        QFontMetrics fmName(option.font);

        QString clippedSymbol = fmName.elidedText(tokenSymbol + labelTypeText, Qt::ElideRight, SYMBOL_WIDTH);
        QRect tokenSymbolRect(mainRect.left() + MARGIN, mainRect.top() + MARGIN, SYMBOL_WIDTH, mainRect.height() / 2 - MARGIN);
        painter->drawText(tokenSymbolRect, Qt::AlignLeft|Qt::AlignVCenter, clippedSymbol);

        int amountWidth = (mainRect.width() - 4 * MARGIN - tokenSymbolRect.width());
        QFontMetrics fmAmount(font);
        QString clippedAmount = fmAmount.elidedText(tokenBalance, Qt::ElideRight, amountWidth);
        QRect tokenBalanceRect(tokenSymbolRect.right() + 2 * MARGIN, tokenSymbolRect.top(), amountWidth, tokenSymbolRect.height());
        painter->drawText(tokenBalanceRect, Qt::AlignLeft|Qt::AlignVCenter, clippedAmount);

        QFont addressFont = option.font;
        addressFont.setPointSizeF(option.font.pointSizeF() * 0.8);
        painter->setFont(addressFont);
        painter->setPen(foreground);
        QRect receiveAddressRect(mainRect.left() + MARGIN, tokenSymbolRect.bottom(), mainRect.width() - 2 * MARGIN, mainRect.height() / 2 - 2 * MARGIN);
        painter->drawText(receiveAddressRect, Qt::AlignLeft|Qt::AlignVCenter, receiveAddress);

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        return QSize(TOKEN_SIZE, TOKEN_SIZE);
    }

    const PlatformStyle *platformStyle;
};

ZRCToken::ZRCToken(const PlatformStyle *platformStyle, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ZRCToken),
    m_model(0),
    m_clientModel(0),
    m_tokenModel(0),
    m_tokenDelegate(0),
    m_tokenTransactionView(0)
{
    ui->setupUi(this);

    m_platformStyle = platformStyle;

    m_sendTokenPage = new SendTokenPage(this);
    m_receiveTokenPage = new ReceiveTokenPage(platformStyle, this);
    m_addTokenPage = new AddTokenPage(this);
    m_tokenDelegate = new TokenViewDelegate(platformStyle, this);

    m_sendTokenPage->setEnabled(false);
    m_receiveTokenPage->setEnabled(false);

    ui->stackedWidgetToken->addWidget(m_sendTokenPage);
    ui->stackedWidgetToken->addWidget(m_receiveTokenPage);
    ui->stackedWidgetToken->addWidget(m_addTokenPage);

    m_tokenTransactionView = new TokenTransactionView(m_platformStyle, this);
    m_tokenTransactionView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->tokenViewLayout->addWidget(m_tokenTransactionView);

    ui->tokensList->setItemDelegate(m_tokenDelegate);
    ui->tokensList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tokensList->setAttribute(Qt::WA_MacShowFocusRect, false);

    QAction *copySenderAction = new QAction(tr("Copy receive address"), this);
    QAction *copyTokenBalanceAction = new QAction(tr("Copy token balance"), this);
    QAction *copyTokenNameAction = new QAction(tr("Copy token name"), this);
    QAction *copyTokenAddressAction = new QAction(tr("Copy contract address"), this);
    QAction *removeTokenAction = new QAction(tr("Remove token"), this);

    contextMenu = new QMenu(ui->tokensList);
    contextMenu->addAction(copySenderAction);
    contextMenu->addAction(copyTokenBalanceAction);
    contextMenu->addAction(copyTokenNameAction);
    contextMenu->addAction(copyTokenAddressAction);
    contextMenu->addAction(removeTokenAction);

    connect(copyTokenAddressAction, &QAction::triggered, this, &ZRCToken::copyTokenAddress);
    connect(copyTokenBalanceAction, &QAction::triggered, this, &ZRCToken::copyTokenBalance);
    connect(copyTokenNameAction, &QAction::triggered, this, &ZRCToken::copyTokenName);
    connect(copySenderAction, &QAction::triggered, this, &ZRCToken::copySenderAddress);
    connect(removeTokenAction, &QAction::triggered, this, &ZRCToken::removeToken);

    connect(ui->tokensList, &QListView::clicked, this, &ZRCToken::on_currentTokenChanged);

    connect(ui->tokensList, &QListView::doubleClicked, this, &ZRCToken::on_tokenDoubleClicked);

    connect(ui->tokensList, &QListView::customContextMenuRequested, this, &ZRCToken::contextualMenu);

    on_goToSendTokenPage();

}

void ZRCToken::on_tokenDoubleClicked(QModelIndex index)
{
    if(m_tokenModel)
    {
        if(index.isValid())
        {
            QString contractAddress;
	    contractAddress = m_tokenModel->data(index, TokenItemModel::AddressRole).toString();

    	    QString contractType;
    	    contractType = m_tokenModel->data(index, TokenItemModel::TypeRole).toString();

	    QString wallet;
    	    wallet = m_tokenModel->data(index, TokenItemModel::SenderRole).toString();

    	    if(contractType.size() != 0)
	    {
		QList<TokenTransactionRecord> cachedRecords;
    		for(interfaces::TokenTx wtokenTx : m_model->wallet().getTokenTxs())
    		{
    		    cachedRecords.append(TokenTransactionRecord::decomposeTransaction(m_model->wallet(), wtokenTx));
    		}
		TokenListShowDialog *listShow = new TokenListShowDialog(this, &cachedRecords, contractAddress, contractType, wallet);
		listShow->show();
	    }
	}
    }
}

ZRCToken::~ZRCToken()
{
    delete ui;
}

void ZRCToken::setModel(WalletModel *_model)
{
    m_model = _model;
    m_addTokenPage->setModel(m_model);
    m_sendTokenPage->setModel(m_model);
    m_tokenTransactionView->setModel(_model);
    if(m_model && m_model->getTokenItemModel())
    {
        // Sort tokens by symbol
        QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
        TokenItemModel* tokenModel = m_model->getTokenItemModel();
        proxyModel->setSourceModel(tokenModel);
        proxyModel->sort(1, Qt::AscendingOrder);
        m_tokenModel = proxyModel;

        // Set tokens model
        ui->tokensList->setModel(m_tokenModel);
        connect(ui->tokensList->selectionModel(), &QItemSelectionModel::currentChanged, this, &ZRCToken::on_currentChanged);

        // Set current token
        connect(m_tokenModel, &QAbstractItemModel::dataChanged, this, &ZRCToken::on_dataChanged);
        connect(m_tokenModel, &QAbstractItemModel::rowsInserted, this, &ZRCToken::on_rowsInserted);
        if(m_tokenModel->rowCount() > 0)
        {
            QModelIndex currentToken(m_tokenModel->index(0, 0));
            ui->tokensList->setCurrentIndex(currentToken);
            on_currentTokenChanged(currentToken);
        }
    }
}

void ZRCToken::setClientModel(ClientModel *_clientModel)
{
    m_clientModel = _clientModel;
    m_sendTokenPage->setClientModel(_clientModel);
    m_addTokenPage->setClientModel(_clientModel);
}

void ZRCToken::on_goToSendTokenPage()
{
    ui->stackedWidgetToken->setCurrentIndex(0);
}

void ZRCToken::on_goToReceiveTokenPage()
{
    ui->stackedWidgetToken->setCurrentIndex(1);
}

void ZRCToken::on_goToAddTokenPage()
{
    ui->stackedWidgetToken->setCurrentIndex(2);
}

void ZRCToken::on_currentTokenChanged(QModelIndex index)
{
    if(m_tokenModel)
    {
        if(index.isValid())
        {
            m_selectedTokenHash = m_tokenModel->data(index, TokenItemModel::HashRole).toString();
            std::string address = m_tokenModel->data(index, TokenItemModel::AddressRole).toString().toStdString();
            std::string symbol = m_tokenModel->data(index, TokenItemModel::SymbolRole).toString().toStdString();
            std::string sender = m_tokenModel->data(index, TokenItemModel::SenderRole).toString().toStdString();
	    int type;
	    QString role = m_tokenModel->data(index, TokenItemModel::TypeRole).toString();
	    if(role.size() == 0)
	    {
		type = 0;
	    }
	    else
	    {
		type = 1;
	    }
            int8_t decimals = m_tokenModel->data(index, TokenItemModel::DecimalsRole).toInt();
            std::string balance = m_tokenModel->data(index, TokenItemModel::RawBalanceRole).toString().toStdString();
            m_sendTokenPage->setTokenData(address, sender, symbol, decimals, balance, type);
            m_receiveTokenPage->setAddress(QString::fromStdString(sender));
            m_receiveTokenPage->setSymbol(QString::fromStdString(symbol));

            if(!m_sendTokenPage->isEnabled())
                m_sendTokenPage->setEnabled(true);
            if(!m_receiveTokenPage->isEnabled())
                m_receiveTokenPage->setEnabled(true);
        }
        else
        {
            m_sendTokenPage->setEnabled(false);
            m_receiveTokenPage->setEnabled(false);
            m_receiveTokenPage->setAddress(QString::fromStdString(""));
            m_receiveTokenPage->setSymbol(QString::fromStdString(""));
        }
    }
}

void ZRCToken::on_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    Q_UNUSED(bottomRight);
    Q_UNUSED(roles);

    if(m_tokenModel)
    {
        QString tokenHash = m_tokenModel->data(topLeft, TokenItemModel::HashRole).toString();
        if(m_selectedTokenHash.isEmpty() ||
                tokenHash == m_selectedTokenHash)
        {
            on_currentTokenChanged(topLeft);
        }
    }
}

void ZRCToken::on_currentChanged(QModelIndex current, QModelIndex previous)
{
    Q_UNUSED(previous);

    on_currentTokenChanged(current);
}

void ZRCToken::on_rowsInserted(QModelIndex index, int first, int last)
{
    Q_UNUSED(index);
    Q_UNUSED(first);
    Q_UNUSED(last);

    if(m_tokenModel->rowCount() == 1)
    {
        QModelIndex currentToken(m_tokenModel->index(0, 0));
        ui->tokensList->setCurrentIndex(currentToken);
        on_currentTokenChanged(currentToken);
    }
}

void ZRCToken::contextualMenu(const QPoint &point)
{
    QModelIndex index = ui->tokensList->indexAt(point);
    QModelIndexList selection = ui->tokensList->selectionModel()->selectedIndexes();
    if (selection.empty())
        return;

    if(index.isValid())
    {
        contextMenu->exec(QCursor::pos());
    }
}

void ZRCToken::copyTokenAddress()
{
    GUIUtil::copyEntryDataFromList(ui->tokensList, TokenItemModel::AddressRole);
}

void ZRCToken::copyTokenBalance()
{
    GUIUtil::copyEntryDataFromList(ui->tokensList, TokenItemModel::BalanceRole);
}

void ZRCToken::copyTokenName()
{
    GUIUtil::copyEntryDataFromList(ui->tokensList, TokenItemModel::NameRole);
}

void ZRCToken::copySenderAddress()
{
    GUIUtil::copyEntryDataFromList(ui->tokensList, TokenItemModel::SenderRole);
}

void ZRCToken::removeToken()
{
    QMessageBox::StandardButton btnRetVal = QMessageBox::question(this, tr("Confirm token remove"), tr("The selected token will be removed from the list. Are you sure?"),
        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);

    if(btnRetVal == QMessageBox::Yes)
    {
        QModelIndexList selection = ui->tokensList->selectionModel()->selectedIndexes();
        if (selection.empty() && !m_model)
            return;

        QModelIndex index = selection[0];
        std::string sHash = index.data(TokenItemModel::HashRole).toString().toStdString();
        m_model->wallet().removeTokenEntry(sHash);
    }
}
