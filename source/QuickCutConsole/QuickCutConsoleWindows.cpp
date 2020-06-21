
#include "pch.h"
#include "QuickCutConsoleWindows.h"
#include "Managers/ProfileManager.h"

// The GUI will send this pattern of key codes to notify that profile changes has been made
// so it knows when to reload the profile data.
#define RESERVED_RELOAD_KEY "86878687"

#define KEY_WAS_DOWN_MASK 0x80
#define KEY_IS_DOWN_MASK  0x01

HHOOK QuickCutConsoleWindows::s_Hook = nullptr;

QuickCutConsoleWindows::QuickCutConsoleWindows(int argc, char * argv[])
    : QuickCutConsole(argc, argv)
{
    s_Instance = this;
}

QuickCutConsoleWindows::~QuickCutConsoleWindows()
{
    if (s_Hook)
    {
        qDebug() << "[QuickCutConsoleWindows::dtor] - Unhooking...";
        UnhookWindowsHookEx(s_Hook);
        s_Hook = nullptr;
    }
}

LRESULT CALLBACK QuickCutConsoleWindows::WndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0) return CallNextHookEx(s_Hook, nCode, wParam, lParam);

    KBDLLHOOKSTRUCT * kbd = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);

    static BYTE    keys[256] = {0};
    static QString pressedKeys;
    static bool    keysAlreadyProcessed = false;

    // Workaround for auto-repeat, since low level hook doesn't provide KF_REPEAT flags in
    // lParam: (lParam & KF_REPEAT)
    static DWORD prevVkCode = 0;

    if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
    {
        if (prevVkCode == kbd->vkCode) return CallNextHookEx(s_Hook, nCode, wParam, lParam);

        prevVkCode = kbd->vkCode;

        if (keysAlreadyProcessed)
        {
            pressedKeys.clear();
            keysAlreadyProcessed = false;
        }

        keys[kbd->vkCode] = KEY_WAS_DOWN_MASK | KEY_IS_DOWN_MASK;

        pressedKeys += QString::number(kbd->vkCode, 16);
        printKeyName(kbd, pressedKeys);

        if (pressedKeys == RESERVED_RELOAD_KEY)
        {
            loadProfiles();
            qDebug() << "Refresh signal requested. Reloading profiles.";
            pressedKeys.clear();
            keysAlreadyProcessed = false;
            prevVkCode           = 0;
            return CallNextHookEx(s_Hook, nCode, wParam, lParam);
        }

        if (!s_Profile) { return CallNextHookEx(s_Hook, nCode, wParam, lParam); }

        for (auto && action : s_Profile->getActionManager())
        {
            if (pressedKeys.toStdString() == action->getSrcKey())
            {
                qDebug() << "Pressed Keys Match!: " << pressedKeys
                         << " | Actual Keys: " << QString::fromStdString(action->getSrcKey());
                eActionType actionType = action->getType();
                if (actionType == ActionKeyMap)
                {
                    qDebug() << "Mapping key -> " << pressedKeys << " To -> "
                             << QString::fromStdString(action->getDstKey());
                    pressedKeys.clear(); // Make sure to clear keys before sending another key.
                    static int vkDstCode = 0;
                    vkDstCode = std::strtol(action->getDstKey().c_str(), nullptr, 16);

                    INPUT in  = {0};
                    in.type   = INPUT_KEYBOARD;
                    in.ki.wVk = vkDstCode;
                    SendInput(1, &in, sizeof(INPUT));

                    return -1; // Don't process the source input.
                }
                else if (actionType == ActionAppStart)
                {
                    qDebug() << "Running process -> "
                             << QString::fromStdString(action->getAppPath()) << " With key -> "
                             << pressedKeys;
                    executeProcess(action->getAppPath(), action->getAppArgs());
                }
            }
        } // end for
    }

    if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
    {
        keys[kbd->vkCode] = KEY_WAS_DOWN_MASK | KEY_IS_DOWN_MASK;

        if (!keysAlreadyProcessed)
        {
            prevVkCode           = 0;
            keysAlreadyProcessed = true;
        }
    }

    return CallNextHookEx(s_Hook, nCode, wParam, lParam);
}

bool QuickCutConsoleWindows::isKeyDown(BYTE key)
{
    return ((key & KEY_IS_DOWN_MASK) == KEY_IS_DOWN_MASK);
}

void QuickCutConsoleWindows::printKeyName(KBDLLHOOKSTRUCT * kbd, const QString & pressedKeys)
{
    DWORD kbdMsg = 1;
    kbdMsg += kbd->scanCode << 16; // Scan code.
    kbdMsg += kbd->flags << 24;    // Extended-key flag.

    char keyName[256]{};
    GetKeyNameText(kbdMsg, reinterpret_cast<LPTSTR>(keyName), sizeof(keyName));
    sprintf_s(keyName, "ScanCode: %d | VirtualKey: 0x%02X | KeyName: %s | Current Keys: %s",
              kbd->scanCode, kbd->vkCode,
              QString::fromUtf16(reinterpret_cast<ushort *>(keyName)).toStdString().c_str(),
              pressedKeys.toStdString().c_str());
    qDebug() << keyName;
}

bool QuickCutConsoleWindows::start()
{
    if (!QuickCutConsole::start())
    {
        qDebug() << "[QuickCutConsoleWindows::start] - Profiles file hasn't been loaded yet.";
    }

    if (!s_Hook)
    {
        qDebug() << "[QuickCutConsoleWindows::start] - Hooking...";
        s_Hook = SetWindowsHookEx(WH_KEYBOARD_LL, WndProc, nullptr, 0);
        if (!s_Hook)
        {
            qDebug() << "[QuickCutConsoleWindows::start] - Hook failed...";
            return false;
        }
    }

    return true;
}

bool QuickCutConsoleWindows::stop()
{
    QuickCutConsole::stop();

    if (s_Hook)
    {
        qDebug() << "[QuickCutConsoleWindows::stop] - Unhooking...";
        UnhookWindowsHookEx(s_Hook);
        s_Hook = nullptr;
    }

    return true;
}
