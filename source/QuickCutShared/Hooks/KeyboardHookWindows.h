
#pragma once

#include "KeyboardHook.h"

#include <Windows.h>

class KeyboardHookWindows : public KeyboardHook
{
    Q_OBJECT
public:
    KeyboardHookWindows(bool multiShortcuts, QObject * parent = nullptr);
    ~KeyboardHookWindows();

    bool activateHook() override;
    bool deactivateHook() override;

private:
    static LRESULT CALLBACK SysKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    static QStringList      getKeysData(const QVector<KBDLLHOOKSTRUCT> & pressedKeys);
    static QString          mapMissingKeyName(const int key);

private:
    static HHOOK s_Hook;
};