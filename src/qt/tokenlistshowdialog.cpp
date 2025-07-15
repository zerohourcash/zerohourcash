#include <qt/tokenlistshowdialog.h>
#include <qt/forms/ui_tokenlistshowdialog.h>

#include <qt/transactiontablemodel.h>
#include <qt/styleSheet.h>

#include <algorithm>

#include <qt/tokenshowdetails.h>

#include <QModelIndex>
#include <QList>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QImageReader>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QBuffer>
#include <QPushButton>
#include <QScrollArea>

#include <QMouseEvent>

TokenListShowDialog::TokenListShowDialog(QWidget *parent, QList<TokenTransactionRecord> *t_records, QString contractAddress, QString contractType, QString wallet) :
    QDialog(parent),
    ui(new Ui::TokenListShowDialog)
{
    ui->setupUi(this);

    SetObjectStyleSheet(this, StyleSheetNames::ScrollBarDark);

    ui->scrollArea->setWidgetResizable(true);
    ui->scrollAreaWidgetContents->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    ui->verticalLayout->addWidget(label);

    ui->verticalLayoutLoad->setAlignment(Qt::AlignHCenter);
    ui->labelLoad->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    ui->labelLoadCurrent->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    ui->gridLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    clock_timer->start(300);
    QObject::connect(clock_timer, SIGNAL(timeout()), this, SLOT(spinnerLoader()));

    if(!t_records->isEmpty())
    {

	qSort(t_records->begin(), t_records->end(), sortByTime);

	QList<int> *intResult = new QList<int>();
	QList<TokenTransactionRecord> *listResult = new QList<TokenTransactionRecord>();

	for(int i = 0; i < t_records->size(); i++)
	{

	    TokenTransactionRecord record = t_records->at(i);

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

		if(QString::fromStdString(record.receiverWallet) == wallet)
		{
		    if(tempCredit > 0 && tempDebit == 0)
		    {
			temp = tempCredit;
		    }
		}

		if(QString::fromStdString(record.senderWallet) == wallet)
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

	while (QLayoutItem *item = ui->gridLayout->layout()->takeAt(0))
	{
    	    delete item->widget();
    	    delete item;
	}

	countLoader = 0;
	countLoaderMax = listResult->size();
	row = 0;
	column = 0;

	for(int i = 0; i < listResult->size(); i++)
	{
	    TokenTransactionRecord record = listResult->at(i);

	    QString tokenId = BitcoinUnits::formatTokenWithUnit("", record.decimals, record.credit, false);

	    if(QString::fromStdString(record.contractAddress) == contractAddress && record.credit > 0 && record.contractType == "1")
	    {
		QString tokenURI = QString::fromStdString(record.tokenURI);
		QJsonParseError errorPtr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(tokenURI.replace("'", "\"").toUtf8(), &errorPtr);
		QJsonObject jsonObject = jsonDoc.object();
		QJsonValue ipfs_url = jsonObject["ipfs_url"];

		if(ipfs_url.toString().size() == 46)
		{
		    QString requestUrl = "https://ipfs.zh.cash/ipfs/" + ipfs_url.toString();

		    QNetworkRequest request;
		    QSslConfiguration conf = request.sslConfiguration();
		    conf.setPeerVerifyMode(QSslSocket::VerifyNone);

		    request.setSslConfiguration(conf);
		    request.setUrl(QUrl(requestUrl));

		    manager = new QNetworkAccessManager();
		    QObject::connect(manager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *reply)
		    {

			QString labelText = QString::fromStdString(record.tokenSymbol) + " ID: " + tokenId;
			QByteArray dataArray = reply->readAll();

			QImageReader imageReader(dataArray.data());
			bool loadOk = true;
			QImage img(240, 170, QImage::Format_ARGB32);

			if(!img.loadFromData(dataArray))
			{
			    loadOk = false;
			}
			else
			{
			    
			}

			QLabel *symbol = new QLabel(labelText);
			symbol->setAlignment(Qt::AlignCenter);
			QLabel *label = new QLabel();
			QPixmap image(":/icons/not_support");

			image = image.scaled(240, 170, Qt::IgnoreAspectRatio);

			if(loadOk)
			{
			    QPixmap pix = QPixmap::fromImage(squared(img, 240));
			    label->setObjectName(QString::number(i));
			    pixmapDetails->insert(dataArray.toBase64(), i);
			    //pix = pix.scaled(240, 170, Qt::IgnoreAspectRatio);
			    label->setPixmap(pix);
			}
			else
			{
			    label->setPixmap(image);
			}

			QVBoxLayout *layout = new QVBoxLayout();
			layout->setContentsMargins(10, 10, 10, 10);
			symbol->setStyleSheet("QWidget { background: black; }");
			layout->addWidget(label);
			layout->addWidget(symbol);


			if (column == 4)
			{
    		    	    row++;
			    column = 0;
			}

			ui->gridLayout->addLayout(layout, row, column, Qt::AlignHCenter | Qt::AlignTop);
			
			column++;
			countLoader++;
		    });
	    	    manager->get(request);
		}
	    }
	}
    }
}

bool TokenListShowDialog::sortByTime(const TokenTransactionRecord &d1, const TokenTransactionRecord &d2)
{
    return d1.time < d2.time;
}

void TokenListShowDialog::spinnerLoader()
{

    QString loadLabel;
    loadLabel += "Loaded " + QString::fromStdString(std::to_string(countLoader)) + " of " + QString::fromStdString(std::to_string(countLoaderMax)) + " images from IPFS...";
    ui->labelLoadCurrent->setText(loadLabel);

    if(countLoader == countLoaderMax)
    {
        clock_timer->stop();
        ui->labelLoad->hide();
        ui->labelLoadCurrent->hide();
    }

    ui->labelLoad->setPixmap(QIcon(QString(":/movies/spinner-%1").arg(spinnerTickCount, 3, 10, QChar('0'))).pixmap(60, 60));
    spinnerTickCount++;
    if(spinnerTickCount == 35)
    {
	spinnerTickCount = 0;
    }
}


QImage TokenListShowDialog::squared(const QImage &image, int size)
{
    if (!image.isNull())
    {
        if ((image.width() == size) && (image.height() == size))
            return image;
        QImage squaredImage(size, size, QImage::Format_ARGB32);
        squaredImage.fill(QColor(0, 0, 0, 0).rgba());
        int w = image.width(), h = image.height();
        QPainter p(&squaredImage);
        QPoint offset;
        QImage copy = (w > h) ? ((h == size) ? image : image.scaledToHeight(size, Qt::SmoothTransformation)) : ((w == size) ? image : image.scaledToWidth(size, Qt::SmoothTransformation));
        w = copy.width();
        h = copy.height();
        offset.setX((w > h) ? (size - w) / 2 : 0);
        offset.setY((w > h) ? 0 : (size - h) / 2);
        p.drawImage(offset, copy);
        p.end();
        return squaredImage;
    }
    return QImage();
}

void TokenListShowDialog::mouseDoubleClickEvent(QMouseEvent *event)
{

    QWidget *widget = childAt(event->pos());
    if (widget) {
        QLabel *label = qobject_cast<QLabel *>(widget);
        if (label) {
    	    const QPixmap *pix = label->pixmap();
	    if(pix)
	    {
		int index = label->objectName().toInt();
		const QString out = pixmapDetails->key(index);
		if(out != 0)
		{
		    TokenShowDetails *tokenDetails = new TokenShowDetails(this, &out);
		    tokenDetails->show();
		}
	    }
        }

    }
    
}

TokenListShowDialog::~TokenListShowDialog()
{
    delete ui;
}

