// Copyright (c) 2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/bitcoin.h>

#include <QCoreApplication>

#include <functional>
#include <string>

#include <QtPlugin>
//Q_IMPORT_PLUGIN(QJpegPlugin)
//Q_IMPORT_PLUGIN(QICOPlugin)
//Q_IMPORT_PLUGIN(QTgaPlugin)
//Q_IMPORT_PLUGIN(QWbmpPlugin)
//Q_IMPORT_PLUGIN(QWebpPlugin)
//Q_IMPORT_PLUGIN(QICNSPlugin)
//Q_IMPORT_PLUGIN(QTiffPlugin)

/** Translate string to current locale using Qt. */
extern const std::function<std::string(const char*)> G_TRANSLATION_FUN = [](const char* psz) {
    return QCoreApplication::translate("bitcoin-core", psz).toStdString();
};

int main(int argc, char* argv[]) { return GuiMain(argc, argv); }
