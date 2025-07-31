#include <qt/tokenshowdetails.h>
#include <qt/forms/ui_tokenshowdetails.h>

#include <qt/styleSheet.h>

#include <QDebug>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QPushButton>
#include <QScrollArea>

TokenShowDetails::TokenShowDetails(QWidget *parent, const QString *out) :
    QDialog(parent),
    ui(new Ui::TokenShowDetails)
{
    ui->setupUi(this);

    SetObjectStyleSheet(this, StyleSheetNames::ScrollBarDark);
    ui->verticalLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    QLabel *label = new QLabel();
    QImage img;
    QString *inBase = new QString(*out);
    QByteArray bytes = QByteArray::fromBase64(inBase->toUtf8());
    img.loadFromData(bytes);
    label->setPixmap(QPixmap::fromImage(img));
    ui->verticalLayout->addWidget(label);
    
}

TokenShowDetails::~TokenShowDetails()
{
    delete ui;
}

