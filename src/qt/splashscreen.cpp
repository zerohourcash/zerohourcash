// Copyright (c) 2011-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

#include <qt/splashscreen.h>

#include <qt/networkstyle.h>

#include <clientversion.h>
#include <interfaces/handler.h>
#include <interfaces/node.h>
#include <interfaces/wallet.h>
#include <ui_interface.h>
#include <util/system.h>
#include <version.h>

#include <QApplication>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QPainter>
#include <QFile>
#include <QTimer>
#include <cmath>
#include <random>
#include <QFontDatabase>
#include <QFontMetricsF>
#include <QRadialGradient>


SplashScreen::SplashScreen(interfaces::Node& node, Qt::WindowFlags f, const NetworkStyle *networkStyle) :
    QWidget(nullptr, f), curAlignment(0), m_node(node)
{
    // Layout and random trajectories are immutable during the animation.
    artFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    artFont.setPixelSize(16);
    const QFontMetricsF metrics(artFont);
    auto loadArt = [&](const QString& path, ArtLayout& art) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) return;
        art.lines = QString::fromUtf8(file.readAll()).split('\n');
        while (!art.lines.isEmpty() && art.lines.last().trimmed().isEmpty()) art.lines.removeLast();
        while (!art.lines.isEmpty() && art.lines.first().trimmed().isEmpty()) art.lines.removeFirst();
        for (int i = 0; i < art.lines.size(); ++i) {
            if (art.lines[i].trimmed().isEmpty()) continue;
            const QRectF ink = metrics.tightBoundingRect(art.lines[i]).translated(
                0, metrics.ascent() + i * metrics.lineSpacing());
            art.bounds = art.bounds.isNull() ? ink : art.bounds.united(ink);
        }
    };
    loadArt(":/splash/art.txt", artwork);
    loadArt(":/splash/logo.txt", logo);
    const quint32 particleSeed = std::random_device{}();
    auto randomUnit = [](quint32 value) {
        value ^= value >> 16; value *= 0x7feb352dU;
        value ^= value >> 15; value *= 0x846ca68bU;
        value ^= value >> 16;
        return qreal(value) / qreal(0xffffffffU);
    };
    for (int i = 0; i < artwork.lines.size(); ++i) {
        qreal x = 0;
        for (int j = 0; j < artwork.lines[i].size(); ++j) {
            const QString glyph(artwork.lines[i][j]);
            if (!artwork.lines[i][j].isSpace()) {
                const quint32 id = particleSeed ^ (quint32(i) * 65537U + quint32(j));
                particles.append({glyph, QPointF(x, metrics.ascent() + i * metrics.lineSpacing()),
                    metrics.tightBoundingRect(glyph), randomUnit(id ^ 0x174b9321U),
                    randomUnit(id ^ 0x62ac84f1U), randomUnit(id ^ 0x35ab192dU) * 1.8,
                    2.8 + randomUnit(id ^ 0x814239abU) * 1.2});
            }
            x += metrics.width(glyph);
        }
    }
    setWindowTitle(tr(PACKAGE_NAME) + " " + networkStyle->getTitleAddText());
    setAutoFillBackground(true);
    setAttribute(Qt::WA_StyledBackground, true);

    animationClock.start();
    auto animationTimer = new QTimer(this);
    animationTimer->setTimerType(Qt::PreciseTimer);
    animationTimer->setInterval(16);
    connect(animationTimer, &QTimer::timeout, this, [this] { if (isVisible()) update(); });
    animationTimer->start();
    subscribeToCoreSignals();
    installEventFilter(this);
}

SplashScreen::~SplashScreen()
{
    unsubscribeFromCoreSignals();
}

bool SplashScreen::eventFilter(QObject * obj, QEvent * ev) {
    if (ev->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(ev);
        if(keyEvent->key() == Qt::Key_Q) {
            m_node.startShutdown();
        }
    }
    return QObject::eventFilter(obj, ev);
}

void SplashScreen::finish()
{
    /* If the window is minimized, hide() will be ignored. */
    /* Make sure we de-minimize the splashscreen window before hiding */
    if (isMinimized())
        showNormal();
    hide();
    deleteLater(); // No more need for this
}

static void InitMessage(SplashScreen *splash, const std::string &message)
{
    QMetaObject::invokeMethod(splash, "showMessage",
        Qt::QueuedConnection,
        Q_ARG(QString, QString::fromStdString(message)),
        Q_ARG(int, Qt::AlignBottom|Qt::AlignRight),
        Q_ARG(QColor, QColor("#FFFFFF")));
}

static void ShowProgress(SplashScreen *splash, const std::string &title, int nProgress, bool resume_possible)
{
    InitMessage(splash, title + std::string("\n") +
            (resume_possible ? _("(press q to shutdown and continue later)")
                                : _("press q to shutdown")) +
            strprintf("\n%d", nProgress) + "%");
}
#ifdef ENABLE_WALLET
void SplashScreen::ConnectWallet(std::unique_ptr<interfaces::Wallet> wallet)
{
    m_connected_wallet_handlers.emplace_back(wallet->handleShowProgress(std::bind(ShowProgress, this, std::placeholders::_1, std::placeholders::_2, false)));
    m_connected_wallets.emplace_back(std::move(wallet));
}
#endif

void SplashScreen::subscribeToCoreSignals()
{
    // Connect signals to client
    m_handler_init_message = m_node.handleInitMessage(std::bind(InitMessage, this, std::placeholders::_1));
    m_handler_show_progress = m_node.handleShowProgress(std::bind(ShowProgress, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
#ifdef ENABLE_WALLET
    m_handler_load_wallet = m_node.handleLoadWallet([this](std::unique_ptr<interfaces::Wallet> wallet) { ConnectWallet(std::move(wallet)); });
#endif
}

void SplashScreen::unsubscribeFromCoreSignals()
{
    // Disconnect signals from client
    m_handler_init_message->disconnect();
    m_handler_show_progress->disconnect();
    for (const auto& handler : m_connected_wallet_handlers) {
        handler->disconnect();
    }
    m_connected_wallet_handlers.clear();
    m_connected_wallets.clear();
}

void SplashScreen::showMessage(const QString &message, int alignment, const QColor &color)
{
    curMessage = message;
    curAlignment = alignment;
    curColor = color;
    update();
}

void SplashScreen::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor("#181818"));
    // Fixed character cells are essential to preserve this text illustration.
    const QFontMetricsF metrics(artFont);
    const qreal seconds = animationClock.elapsed() / 1000.0;
    const qreal fade = qMin(qreal(1), seconds / 0.65);
    const qreal drift = 3.0 * std::sin(seconds * 0.75);
    painter.setOpacity(fade * (0.98 + 0.02 * std::cos(seconds * 0.6)));
    auto drawArt = [&](const ArtLayout& art, const QRectF& area, bool primary = false) {
        const QRectF& bounds = art.bounds;
        if (bounds.isEmpty() || area.isEmpty()) return QRectF();
        const qreal pulse = primary ? 1.0 : 0.94 + 0.06 * std::sin(seconds * 2.8);
        const qreal scale = qMin(area.width() / bounds.width(),
                                 area.height() / bounds.height()) * pulse;
        const QPointF origin = area.center() - bounds.center() * scale;
        painter.save();
        painter.translate(origin);
        painter.scale(scale, scale);
        painter.setFont(artFont);
        painter.setPen(QColor("#c8cac8"));
        if (!primary || seconds >= 6.0) {
            for (int i = 0; i < art.lines.size(); ++i)
                painter.drawText(QPointF(0, metrics.ascent() + i * metrics.lineSpacing()), art.lines[i]);
        } else {
            for (const Particle& particle : particles) {
                const qreal t = qBound(qreal(0), (seconds - particle.delay) / particle.duration, qreal(1));
                const qreal eased = t * t * t * (t * (t * 6 - 15) + 10);
                const QRectF& ink = particle.ink;
                const QPointF startScreen(
                    particle.randomX * qMax(qreal(0), width() - ink.width() * scale) - ink.left() * scale,
                    particle.randomY * qMax(qreal(0), height() - ink.height() * scale) - ink.top() * scale);
                const QPointF start = (startScreen - origin) / scale;
                painter.drawText(start * (1 - eased) + particle.destination * eased, particle.glyph);
            }
        }
        painter.restore();
        return QRectF(origin + bounds.topLeft() * scale, bounds.size() * scale);
    };
    const QRectF content(rect());
    QRectF logoBounds;
    // Fit the completed artwork edge to edge; particles assemble in screen space.
    if (content.width() > content.height()) {
        const QRectF artArea(content.left(), content.top(), content.width() * 0.72, content.height());
        drawArt(artwork, artArea, true);
        const qreal side = qMin(content.width() * 0.25, content.height() * 0.32);
        const QRectF brand(content.center().x() - side / 2,
                           content.center().y() - side / 2, side, side);
        logoBounds = drawArt(logo, brand.adjusted(4, 4, -4, -4).translated(0, -drift));
    } else {
        const qreal headerHeight = content.height() * 0.22;
        logoBounds = drawArt(logo, QRectF(content.left() + 4, content.top() + 4 - drift,
            content.width() - 8, qMax(qreal(1), headerHeight - 54)));
        const QRectF artArea(content.left(), content.top() + headerHeight,
                             content.width(), content.height() - headerHeight);
        drawArt(artwork, artArea, true);
    }
    QFont brandFont = QApplication::font();
    brandFont.setPixelSize(qBound(18, height() / 40, 30));
    painter.setFont(brandFont);
    painter.setPen(QColor("#e3e3e3"));
    const QFontMetricsF brandMetrics(brandFont);
    const QRectF labelInk = brandMetrics.tightBoundingRect("ZHCASH");
    painter.drawText(QPointF(logoBounds.center().x() - labelInk.center().x() + 20,
                            logoBounds.bottom() + 12 - labelInk.top()), "ZHCASH");
    painter.setOpacity(1.0);
    QRect r = rect().adjusted(24, 24, -24, -24);
    painter.setPen(curColor);
    QFont font = QApplication::font();
    font.setPointSizeF(font.pointSizeF() * 0.9);
    painter.setFont(font);
    painter.drawText(r, curAlignment, curMessage);
}

void SplashScreen::closeEvent(QCloseEvent *event)
{
    m_node.startShutdown(); // allows an "emergency" shutdown during startup
    event->ignore();
}
