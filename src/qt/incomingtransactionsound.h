// Copyright (c) 2026 The ZHCASH developers
// Distributed under the MIT software license; see COPYING.
#ifndef ZHCASH_QT_INCOMINGTRANSACTIONSOUND_H
#define ZHCASH_QT_INCOMINGTRANSACTIONSOUND_H

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QPointer>
#include <QProcess>
#include <QSettings>
#include <QStringList>
#include <QTemporaryFile>

// Same MP3 and 72% volume as ZHC Wallet Desktop. Audio is best-effort and
// asynchronous; failures cannot interfere with transaction processing.
inline void PlayIncomingTransactionSound(const QString& transactionKey)
{
#ifdef Q_OS_MAC
    if (!QSettings().value("fIncomingTransactionSound", true).toBool()) return;
    static QStringList recent;
    if (transactionKey.isEmpty() || recent.contains(transactionKey)) return;
    recent.append(transactionKey);
    if (recent.size() > 256) recent.removeFirst();
    static QPointer<QProcess> playing;
    if (playing && playing->state() != QProcess::NotRunning) return;
    QFile audio(QStringLiteral(":/sounds/cash-register.mp3"));
    if (!audio.open(QIODevice::ReadOnly)) return;
    QProcess* process = new QProcess(QCoreApplication::instance());
    QTemporaryFile* file = new QTemporaryFile(QDir::tempPath() + "/zhcash-deposit-XXXXXX.mp3", process);
    const QByteArray bytes = audio.readAll();
    if (!file->open() || file->write(bytes) != bytes.size() || !file->flush()) {
        delete process; return;
    }
    file->close();
    playing = process;
    QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), process, &QObject::deleteLater);
    QObject::connect(process, &QProcess::errorOccurred, process, [process](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) process->deleteLater();
    });
    process->setStandardOutputFile(QProcess::nullDevice());
    process->setStandardErrorFile(QProcess::nullDevice());
    process->start(QStringLiteral("/usr/bin/afplay"), QStringList() << "-v" << "0.72" << file->fileName());
#else
    Q_UNUSED(transactionKey);
#endif
}
#endif
