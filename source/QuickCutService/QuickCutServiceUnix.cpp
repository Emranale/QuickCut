
#include "pch.h"
#include "QuickCutServiceUnix.h"

QuickCutServiceUnix::QuickCutServiceUnix(int argc, char * argv[])
    : QuickCutService(argc, argv)
{
}

QuickCutServiceUnix::~QuickCutServiceUnix()
{
    if (isProcessRunning("QuickCutConsole"))
        QProcess::execute("killall -9 QuickCutConsole", QStringList());
}

void QuickCutServiceUnix::start()
{
    QuickCutService::start();

    QString process = QCoreApplication::applicationDirPath() + "/QuickCutConsole";
    QProcess::execute(process, QStringList());
}

void QuickCutServiceUnix::pause()
{
    QuickCutService::pause();

    stop();
}

void QuickCutServiceUnix::resume()
{
    QuickCutService::resume();

    start();
}

void QuickCutServiceUnix::stop()
{
    QuickCutService::stop();
    if (isProcessRunning("QuickCutConsole"))
        QProcess::execute("killall -9 QuickCutConsole", QStringList());
}

bool QuickCutServiceUnix::isProcessRunning(const QString & process)
{
    Q_UNUSED(process);
    return false;
}
