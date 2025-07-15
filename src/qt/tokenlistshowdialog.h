#ifndef ZHC_QT_TOKENLISTSHOWDIALOG_H
#define ZHC_QT_TOKENLISTSHOWDIALOG_H

#include <qt/tokentransactionrecord.h>
#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QLabel>
#include <QPixmap>
#include <QTimer>
#include <QPainter>

namespace Ui {
    class TokenListShowDialog;
}

class TokenListShowDialog: public QDialog
{
    Q_OBJECT

public:
    explicit TokenListShowDialog(QWidget *parent = nullptr, QList<TokenTransactionRecord> *t_records = nullptr, QString contractAddress = "", QString contractType = "", QString wallet = "");

    QNetworkAccessManager *manager;
    QLabel *label = new QLabel();

    QTimer *clock_timer = new QTimer();
    QImage squared(const QImage & image, int size);
    QMap<QString, int> *pixmapDetails = new QMap<QString, int>();

    static bool sortByTime(const TokenTransactionRecord &d1, const TokenTransactionRecord &d2);

    int spinnerTickCount = 0;
    int countLoader;
    int countLoaderMax;
    int row = 0;
    int column = 0;

    ~TokenListShowDialog();

public Q_SLOTS:
    void spinnerLoader();
    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    Ui::TokenListShowDialog *ui;

};

#endif // ZHC_QT_TOKENLISTSHOWDIALOG_H
