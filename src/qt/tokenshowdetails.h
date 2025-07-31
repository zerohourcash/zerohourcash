#ifndef ZHC_QT_TOKENSHOWDETAILS_H
#define ZHC_QT_TOKENSHOWDETAILS_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QLabel>
#include <QPixmap>
#include <QTimer>

namespace Ui {
    class TokenShowDetails;
}

class TokenShowDetails: public QDialog
{
    Q_OBJECT

public:
    explicit TokenShowDetails(QWidget *parent = nullptr, const QString *out = nullptr);

    ~TokenShowDetails();

public Q_SLOTS:

private:
    Ui::TokenShowDetails *ui;

};

#endif // ZHC_QT_TOKENSHOWDETAILS_H
