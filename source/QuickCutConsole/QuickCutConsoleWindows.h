
#pragma once

#include "QuickCutConsole.h"

#include <Windows.h>

class QuickCutConsoleWindows : public QuickCutConsole
{

public:
    QuickCutConsoleWindows(int argc, char * argv[]);
    ~QuickCutConsoleWindows();

    bool start() override;
    bool stop() override;

    static LRESULT CALLBACK WndProc(int nCode, WPARAM wParam, LPARAM lParam);
    static bool             isKeyDown(BYTE key);
    static void             printKeyName(KBDLLHOOKSTRUCT * kbd, const QString & pressedKeys);

public:
    static HHOOK s_Hook;
};
