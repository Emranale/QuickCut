
#pragma once

#include "QuickCutShared/QuickCutPCH.h"

#if defined(Q_OS_WINDOWS)
#    include "QuickCutShared/Hooks/KeyboardHookWindows.h"
#else
#    include "QuickCutShared/Hooks/KeyboardHookUnix.h"
#endif

#include <QCoreApplication>

class Profile;
class ProfileManager;
class QLocalServer;

class QuickCutConsole : public QCoreApplication
{

protected:
    QuickCutConsole(int argc, char * argv[]);
    virtual ~QuickCutConsole();

    virtual bool start();
    virtual bool stop();

    static void executeProcess(const QString & process, const QString & arguments);

    static bool loadProfiles();
    static bool notifyStatusToClient(const QString & message);

    static void log(const QString & filePath, const QString & text);

public slots:
    void onKeysPress(const QStringList & keys, bool * outSwallowKey);

protected:
    static QuickCutConsole * s_Instance;

    static Profile *      s_Profile;
    static ProfileManager s_ProfileManager;

    // Will be used for signaling when the GUI updates the profiles.json file, the server will
    // reload the profiles in order to get things synchronized.
    static QLocalServer *  s_LocalSocket;
    QPointer<KeyboardHook> m_Hook;
};
