#include <qt/styleSheet.h>

#include <qt/platformstyle.h>
#include <qt/rpcconsole.h>
#include <memory>
#include <QAbstractButton>
#include <QTemporaryDir>
#include <QFile>
#include <QSettings>
#include <QRegularExpression>
#include <QFontDatabase>
#include <QWidget>
#include <QApplication>
#include <QStyleFactory>
#include <QProxyStyle>
#include <QListView>
#include <QTableView>
#include <QHeaderView>
#include <QComboBox>
#include <QMessageBox>
#include <QPushButton>
#include <QPainter>
#include <QLineEdit>
#include <QtGlobal>

static const QString STYLE_FORMAT = ":/styles/%1";
static const QColor LINK_COLOR = "#e3e3e3";

class ZHCASHStyle : public QProxyStyle
{
public:

    void polish(QWidget *widget)
    {
        if(widget && widget->inherits("QComboBox"))
        {
            QComboBox* comboBox = (QComboBox*)widget;
            if(comboBox->view() && comboBox->view()->inherits("QComboBoxListView"))
            {
                comboBox->setView(new QListView());
                qApp->processEvents();
            }

            if(comboBox->view() && comboBox->view()->parentWidget())
            {
                QWidget* parent = comboBox->view()->parentWidget();
                parent->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
                parent->setAttribute(Qt::WA_TranslucentBackground);
            }
        }
        if(widget && widget->inherits("QMessageBox"))
        {
            QMessageBox* messageBox = (QMessageBox*)widget;
            QPixmap iconPixmap;
            QMessageBox::Icon icon = messageBox->icon();
            switch (icon)
            {
            case QMessageBox::Information:
                iconPixmap = QPixmap(":/styles/app-icons/message_info");
                break;
            case QMessageBox::Warning:
                iconPixmap = QPixmap(":/styles/app-icons/message_warning");
                break;
            case QMessageBox::Critical:
                iconPixmap = QPixmap(":/styles/app-icons/message_critical");
                break;
            case QMessageBox::Question:
                iconPixmap = QPixmap(":/styles/app-icons/message_question");
                break;
            default:
                QProxyStyle::polish(widget);
                return;
            }
            messageBox->setIconPixmap(iconPixmap.scaled(45,49));
        }
        if(widget && widget->inherits("QLineEdit"))
        {
            QLineEdit* lineEdit = (QLineEdit*)widget;
            if(lineEdit->isReadOnly())
            {
                lineEdit->setFocusPolicy(Qt::ClickFocus);
            }
        }

        // UI glyphs loaded directly from .ui files must also remain legible.
        if (auto* button = qobject_cast<QAbstractButton*>(widget)) {
            if (!button->icon().isNull()) {
                static const std::unique_ptr<const PlatformStyle> icons(PlatformStyle::instantiate("macosx"));
                button->setIcon(icons->SingleColorIcon(button->icon()));
            }
        }
        QProxyStyle::polish(widget);
    }
};

StyleSheet &StyleSheet::instance()
{
    static StyleSheet inst;
    return inst;
}


QString StyleSheet::scaleFontSizes(const QString& style, double factor)
{
    // Scale only font declarations, never borders, spacing or icon dimensions.
    const QRegularExpression re("(font(?:-size)?\\s*:[^;{}]*?)([0-9]+(?:\\.[0-9]+)?)(pt|px)");
    QString result;
    int end = 0;
    auto matches = re.globalMatch(style);
    while (matches.hasNext()) {
        const auto match = matches.next();
        result += style.mid(end, match.capturedStart() - end) + match.captured(1)
            + QString::number(match.captured(2).toDouble() * factor, 'f', 2) + match.captured(3);
        end = match.capturedEnd();
    }
    return result + style.mid(end);
}

void StyleSheet::applyFontScale(int percent)
{
    percent = qBound(80, percent, 160);
    const double previous = qApp->property("zhcFontScale").toDouble();
    const double factor = percent / 100.0;
    const double ratio = factor / (previous > 0 ? previous : 1.0);
    if (qFuzzyCompare(ratio, 1.0)) return;
    struct Appearance { QWidget* widget; QFont font; QString style; };
    QList<Appearance> appearances;
    for (QWidget* widget : QApplication::allWidgets())
        appearances.append({widget, widget->font(), widget->styleSheet()});
    QFont font = qApp->font();
    if (font.pointSizeF() > 0) font.setPointSizeF(font.pointSizeF() * ratio);
    else font.setPixelSize(qMax(1, qRound(font.pixelSize() * ratio)));
    qApp->setFont(font);
    qApp->setStyleSheet(scaleFontSizes(qApp->styleSheet(), ratio));
    for (const auto& appearance : appearances) {
        QFont scaled = appearance.font;
        if (scaled.pointSizeF() > 0) scaled.setPointSizeF(scaled.pointSizeF() * ratio);
        else scaled.setPixelSize(qMax(1, qRound(scaled.pixelSize() * ratio)));
        appearance.widget->setFont(scaled);
        if (!appearance.style.isEmpty())
            appearance.widget->setStyleSheet(scaleFontSizes(appearance.style, ratio));
        if (auto table = qobject_cast<QTableView*>(appearance.widget)) {
            table->resizeColumnsToContents();
            table->verticalHeader()->setDefaultSectionSize(qMax(20,
                qRound(table->verticalHeader()->defaultSectionSize() * ratio)));
        }
        appearance.widget->updateGeometry();
        appearance.widget->update();
    }
    QSettings settings;
    const int consoleSize = qBound(4, qRound(settings.value("consoleFontSize",
        qApp->font().pointSizeF() / ratio).toDouble() * ratio), 40);
    for (const auto& appearance : appearances) {
        if (auto console = qobject_cast<RPCConsole*>(appearance.widget))
            console->setFontSize(consoleSize);
    }
    settings.setValue("consoleFontSize", consoleSize);
    qApp->setProperty("zhcFontScale", factor);
}

StyleSheet::StyleSheet()
{}

void StyleSheet::setStyleSheet(QWidget *widget, const QString &style_name)
{
    setObjectStyleSheet<QWidget>(widget, style_name);
}

void StyleSheet::setStyleSheet(QApplication *app, const QString& style_name)
{
    QStyle* mainStyle = QStyleFactory::create("fusion");
    ZHCASHStyle* zerohourStyle = new ZHCASHStyle;
    zerohourStyle->setBaseStyle(mainStyle);
    app->setStyle(zerohourStyle);

    QPalette mainPalette(app->palette());
    mainPalette.setColor(QPalette::Link, LINK_COLOR);
    mainPalette.setColor(QPalette::Window, QColor("#181818"));
    mainPalette.setColor(QPalette::WindowText, QColor("#e3e3e3"));
    mainPalette.setColor(QPalette::Base, QColor("#353535"));
    mainPalette.setColor(QPalette::AlternateBase, QColor("#2e302e"));
    mainPalette.setColor(QPalette::Text, QColor("#e3e3e3"));
    mainPalette.setColor(QPalette::Button, QColor("#353535"));
    mainPalette.setColor(QPalette::ButtonText, QColor("#e3e3e3"));
    mainPalette.setColor(QPalette::Highlight, QColor("#3b3e3b"));
    mainPalette.setColor(QPalette::HighlightedText, QColor("#ededed"));
    mainPalette.setColor(QPalette::ToolTipBase, QColor("#292c29"));
    mainPalette.setColor(QPalette::ToolTipText, QColor("#e3e3e3"));
    mainPalette.setColor(QPalette::Disabled, QPalette::Text, QColor("#777a77"));
    mainPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#777a77"));
    app->setPalette(mainPalette);

    QFont font = app->font();
#ifdef Q_OS_MAC
    font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    font.setPointSizeF(13.0);
    font.setWeight(QFont::Normal);
    font.setKerning(true);
#else
    font.setPointSizeF(font.pointSizeF() * 1.1);
#endif
    const double scale = qBound(80, QSettings().value("uiFontScale", 100).toInt(), 160) / 100.0;
    font.setPointSizeF(font.pointSizeF() * scale);
    app->setProperty("zhcFontScale", scale);
    app->setFont(font);

    setObjectStyleSheet<QApplication>(app, style_name);
}

QString StyleSheet::getStyleSheet(const QString &style_name)
{
    QString style;
    QFile file(STYLE_FORMAT.arg(style_name));
    if(file.open(QIODevice::ReadOnly))
    {
        style = file.readAll();
        // QSS image URLs bypass QIcon tinting. Cache light arrow masks for this process.
        static QTemporaryDir glyphDirectory;
        const QStringList arrows = {"up_arrow", "down_arrow", "up_arrow_hover", "down_arrow_hover",
            "up_arrow_disabled", "down_arrow_disabled", "toolbutton_down_arrow"};
        for (const QString& name : arrows) {
            const QString resource = ":/styles/app-icons/" + name;
            if (!style.contains(resource) || !glyphDirectory.isValid()) continue;
            const QString path = glyphDirectory.path() + "/" + name + ".png";
            if (!QFile::exists(path)) {
                QImage glyph(resource);
                glyph = glyph.convertToFormat(QImage::Format_ARGB32);
                int maxAlpha = 0;
                for (int y = 0; y < glyph.height(); ++y)
                    for (int x = 0; x < glyph.width(); ++x)
                        maxAlpha = qMax(maxAlpha, qAlpha(glyph.pixel(x, y)));
                const int shade = name.contains("disabled") ? 145 : 227;
                for (int y = 0; y < glyph.height(); ++y)
                    for (int x = 0; x < glyph.width(); ++x)
                        glyph.setPixel(x, y, qRgba(shade, shade, shade,
                            maxAlpha ? qAlpha(glyph.pixel(x, y)) * 255 / maxAlpha : 0));
                if (!glyph.save(path)) continue;
            }
            // Match a complete URL so a normal arrow never consumes a hover suffix.
            style.replace(resource + "\"", path + "\"");
        }
        m_cacheStyles[style_name] = style;
    }
    return style;
}

template<typename T>
void StyleSheet::setObjectStyleSheet(T *object, const QString &style_name)
{
    QString style_value = m_cacheStyles.contains(style_name) ? m_cacheStyles[style_name] : getStyleSheet(style_name);
    const double scale = qApp->property("zhcFontScale").toDouble();
    object->setStyleSheet(scaleFontSizes(style_value, scale > 0 ? scale : 1.0));
}
