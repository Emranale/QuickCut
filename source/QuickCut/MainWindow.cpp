﻿
#include "pch.h"
#include "MainWindow.h"
#include "Types.h"
#include "Utils/Utility.h"

#include <QThread>
#include <QTimer>
#include <QInputDialog>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>

#include <QtService/QtService>

#include "ActionEditWindow.h"
#include "AboutWindow.h"
#include "CheckUpdatesWindow.h"
#include "ExamplesWindow.h"

MainWindow::MainWindow(QWidget * parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowClass)
    , m_ActionEditWindow(nullptr)
    , m_AboutWindow(nullptr)
    , m_CheckUpdatesWindow(nullptr)
    , m_ExamplesWindow(nullptr)
    , m_Profiles("Config/profiles.json")
    , m_Preference("Config/preference.json")
{
    ui->setupUi(this);

    initPreference();
    initProfiles();
    activateHook();

    connectSlots();
    statusBar()->showMessage("Ready.");
}

MainWindow::~MainWindow()
{
    m_Profiles.clear();
}

void MainWindow::connectSlots()
{
    // File Menu
    connect(ui->actionFileOpen, &QAction::triggered, this, &MainWindow::onActionFileOpen);
    connect(ui->actionFileSave, &QAction::triggered, this, &MainWindow::onActionFileSave);
    connect(ui->actionFileSaveAs, &QAction::triggered, this, &MainWindow::onActionFileSaveAs);
    connect(ui->actionFileRestartService, &QAction::triggered, this,
            &MainWindow::onActionFileRestartService);
    connect(ui->actionFileExit, &QAction::triggered, this, &MainWindow::onActionFileExit);

    // View Menu
    connect(ui->actionViewToolBar, &QAction::triggered, this,
            &MainWindow::onActionViewToolBar);
    connect(ui->actionViewStatusBar, &QAction::triggered, this,
            &MainWindow::onActionViewStatusBar);
    connect(ui->actionViewRefresh, &QAction::triggered, this,
            &MainWindow::onActionViewRefresh);

    // Preference Menu
    connect(ui->actionThemeDefault, &QAction::triggered, this,
            [this] { onActionLoadTheme(ThemeDefault, ui->actionThemeDefault); });
    connect(ui->actionThemeDark, &QAction::triggered, this,
            [this] { onActionLoadTheme(ThemeDark, ui->actionThemeDark); });
    connect(ui->actionThemeBreezeDark, &QAction::triggered, this,
            [this] { onActionLoadTheme(ThemeBreezeDark, ui->actionThemeBreezeDark); });
    connect(ui->actionThemeBreezeLight, &QAction::triggered, this,
            [this] { onActionLoadTheme(ThemeBreezeLight, ui->actionThemeBreezeLight); });
    connect(ui->actionThemeConsoleDark, &QAction::triggered, this,
            [this] { onActionLoadTheme(ThemeConsoleDark, ui->actionThemeConsoleDark); });
    connect(ui->actionThemeUbuntu, &QAction::triggered, this,
            [this] { onActionLoadTheme(ThemeUbuntu, ui->actionThemeUbuntu); });
    connect(ui->actionThemeDarkOrange, &QAction::triggered, this,
            [this] { onActionLoadTheme(ThemeDarkOrange, ui->actionThemeDarkOrange); });
    m_ThemeActions.insert(ThemeDefault, ui->actionThemeDefault);
    m_ThemeActions.insert(ThemeDark, ui->actionThemeDark);
    m_ThemeActions.insert(ThemeBreezeDark, ui->actionThemeBreezeDark);
    m_ThemeActions.insert(ThemeBreezeLight, ui->actionThemeBreezeLight);
    m_ThemeActions.insert(ThemeConsoleDark, ui->actionThemeConsoleDark);
    m_ThemeActions.insert(ThemeUbuntu, ui->actionThemeUbuntu);
    m_ThemeActions.insert(ThemeDarkOrange, ui->actionThemeDarkOrange);

    connect(ui->actionThemeLoadQss, &QAction::triggered, this,
            &MainWindow::onLoadCustomStylesheet);

    // Help Menu
    connect(ui->actionHelpAbout, &QAction::triggered, this, &MainWindow::onActionHelpAbout);
    connect(ui->actionHelpExamples, &QAction::triggered, this,
            &MainWindow::onActionHelpExamples);
    connect(ui->actionHelpCheckUpdates, &QAction::triggered, this,
            &MainWindow::onActionHelpCheckUpdates);

    // Controls
    connect(ui->lbxActions, &QListWidget::currentRowChanged, this,
            &MainWindow::onActionSelChange);
    connect(ui->lbxActions, &QListWidget::doubleClicked, this,
            &MainWindow::onActionDoubleClicked);
    connect(ui->cbxProfile, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &MainWindow::onProfileSelChange);

    connect(ui->btnSetActive, &QPushButton::clicked, this, &MainWindow::onBtnSetActiveProfile);
    connect(ui->btnCreateProfile, &QPushButton::clicked, this,
            &MainWindow::onBtnCreateProfile);
    connect(ui->btnDeleteProfile, &QPushButton::clicked, this,
            &MainWindow::onBtnDeleteProfile);

    connect(ui->btnActionCreate, &QPushButton::clicked, this, &MainWindow::onBtnActionCreate);
    connect(ui->btnActionDelete, &QPushButton::clicked, this, &MainWindow::onBtnActionDelete);
    connect(ui->btnActionDuplicate, &QPushButton::clicked, this,
            &MainWindow::onBtnActionDuplicate);

    connect(ui->btnActionMoveDown, &QPushButton::clicked, this,
            &MainWindow::onBtnActionMoveDown);
    connect(ui->btnActionMoveUp, &QPushButton::clicked, this, &MainWindow::onBtnActionMoveUp);
}

void MainWindow::activateHook()
{
    QProcess::execute("QuickCutService -i", QStringList()); // TODO: Configure from installer.
    QtServiceController service("QuickCut Service");
    if (!service.isRunning())
    {
        service.start();
        ui->statusBar->showMessage("Activated QuickCut Service.");
    }

#if defined(Q_OS_WIN)
    // TODO: Configure from installer.
    // Don't start the service right when win boots.
    QProcess::execute("sc config \"QuickCut Service\" start=delayed-auto", QStringList());
    QProcess::execute("sc failure \"QuickCut Service\" actions=restart/60000/restart/60000/"
                      "/60000 reset=86400",
                      QStringList());

    const QString qcc = QCoreApplication::applicationDirPath() + "/QuickCutConsole.exe";

    // Approach 1:
    // QProcess::execute("schtasks /Create /TN \"QuickCut\" /SC ONLOGON /TR \"" + qcc + "\"");

    // Approach 2:
    // QSettings
    // settings(R"(HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Run)",
    // QSettings::NativeFormat); const QString value(settings.value("QuickCutConsole",
    // QString()).toString()); if (value.isEmpty() || value != qcc)
    //    settings.setValue("QuickCutConsole", qcc);

    // Approach 3: <- Simple and works.
    QDir dir(QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).first());
    dir.cdUp();
    QFile::link(qcc, dir.path() +
                         "/Microsoft/Windows/Start Menu/Programs/Startup/QuickCutConsole.lnk");

    // Windows service starts the console hook as the default user using the winsta0\\default
    // which sometimes fails(QuickCutServiceWindows::RunProcessAsUserW), so if the user starts
    // QuickCut GUI, we use the chance to start QuickCutConsole as the current user just in
    // case.
    QTimer::singleShot(5000, [] {
        QThread * t = QThread::create([] {
            QProcess::execute("taskkill /im QuickCutConsole.exe /f", QStringList());
            QThread::msleep(500);
            QProcess::execute("QuickCutConsole", QStringList());
        });
        t->start();
    });
#endif
}

void MainWindow::showEvent(QShowEvent * event)
{
    QMainWindow::showEvent(event);
}

void MainWindow::initPreference()
{

    if (!m_Preference.load()) m_Preference.save();

    QAction * action = nullptr;
    switch (m_Preference.get().getCurrentTheme())
    {
        case ThemeDefault:
            action = ui->actionThemeDefault;
            break;
        case ThemeDark:
            action = ui->actionThemeDark;
            break;
        case ThemeBreezeDark:
            action = ui->actionThemeBreezeDark;
            break;
        case ThemeBreezeLight:
            action = ui->actionThemeBreezeLight;
            break;
        case ThemeConsoleDark:
            action = ui->actionThemeConsoleDark;
            break;
        case ThemeUbuntu:
            action = ui->actionThemeUbuntu;
            break;
        case ThemeDarkOrange:
            action = ui->actionThemeDarkOrange;
            break;
        default:
            break;
    }

    onActionLoadTheme(m_Preference.get().getCurrentThemeResourcePath(), action);

    ui->actionViewToolBar->setChecked(m_Preference.get().isToolBarVisible());
    ui->actionViewStatusBar->setChecked(m_Preference.get().isStatusBarVisible());
    ui->toolBar->setVisible(m_Preference.get().isToolBarVisible());
    ui->statusBar->setVisible(m_Preference.get().isStatusBarVisible());
}

void MainWindow::initProfiles()
{
    ui->lbxActions->clear();
    ui->cbxProfile->clear();

    if (!m_Profiles.load() || m_Profiles.empty())
    {
        Profile * profile = onBtnCreateProfile();
        if (profile)
            m_Profiles.setActiveProfile(profile);
        else
            onProfileSelChange(-1);

        onActionSelChange(-1);
        saveProfiles();
    }
    else
    {
        for (auto && profile : m_Profiles) ui->cbxProfile->addItem(profile->getName());

        Profile * profile = m_Profiles.getActiveProfile();
        if (profile)
        {
            ui->cbxProfile->setCurrentText(profile->getName());
            if (!profile->getActionManager().empty())
            {
                for (auto && action : profile->getActionManager())
                    ui->lbxActions->addItem(action->getName());
                onActionSelChange(0);
            }
            else
                onActionSelChange(-1);
        }
    }

    ui->btnSetActive->setEnabled(false);
}

bool MainWindow::reloadProfiles()
{
    if (!loadProfiles()) return false;

    onProfileSelChange(ui->cbxProfile->currentIndex());

    return true;
}

bool MainWindow::loadProfiles()
{
    return m_Profiles.load();
}

bool MainWindow::saveProfiles()
{
    bool result = m_Profiles.save();
    if (result) Hook::sendReloadSignal();
    return result;
}

void MainWindow::onProfileSelChange(int index)
{
    ui->lbxActions->clear();
    Profile * profile = m_Profiles[index];
    if (!profile)
    {
        ui->btnSetActive->setEnabled(false);
        ui->btnDeleteProfile->setEnabled(false);
        ui->btnActionCreate->setEnabled(false);
        ui->btnActionDelete->setEnabled(false);
        ui->btnActionDuplicate->setEnabled(false);
        return;
    }

    ui->btnSetActive->setEnabled(!profile->isActive());
    ui->btnDeleteProfile->setEnabled(true);
    ui->btnActionCreate->setEnabled(true);

    disconnect(ui->lbxActions, &QListWidget::currentRowChanged, this,
               &MainWindow::onActionSelChange);
    for (auto && action : profile->getActionManager())
        ui->lbxActions->addItem(action->getName());
    connect(ui->lbxActions, &QListWidget::currentRowChanged, this,
            &MainWindow::onActionSelChange);

    if (profile->getActionManager().empty())
        onActionSelChange(-1);
    else
        ui->lbxActions->setCurrentRow(0);
}

void MainWindow::onBtnSetActiveProfile()
{
    Profile * profile = m_Profiles[ui->cbxProfile->currentIndex()];
    if (!profile) return;

    m_Profiles.setActiveProfile(profile);
    saveProfiles();
    reloadProfiles();
}

void MainWindow::onBtnDeleteProfile()
{
    const int currIndex = ui->cbxProfile->currentIndex();
    Profile * profile   = m_Profiles[currIndex];

    bool wasActiveProfile = profile->isActive();
    m_Profiles.remove(currIndex);
    ui->cbxProfile->removeItem(currIndex);
    if (wasActiveProfile && m_Profiles.count() > 0)
    {
        m_Profiles.setActiveProfile(0);
        ui->cbxProfile->setCurrentIndex(0);
    }
    saveProfiles();
    reloadProfiles();
}

Profile * MainWindow::onBtnCreateProfile()
{
    bool    ok;
    QString profileName = QInputDialog::getText(
        this, tr("Create Profile"), tr("Profile Name:"), QLineEdit::Normal,
        QDir::home().dirName(), &ok,
        Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);

    if (!ok && profileName.isEmpty()) return nullptr;

    Profile * profile = m_Profiles.getByName(profileName);
    if (profile)
    {
        QMessageBox::information(this, "Already Exists",
                                 "The given profile name already exists.",
                                 QMessageBox::StandardButton::Ok);
        return nullptr;
    }

    profile = new Profile();
    profile->setName(profileName);
    if (m_Profiles.empty()) profile->setActive(true);
    m_Profiles.add(profile);
    saveProfiles();

    ui->cbxProfile->addItem(profileName);
    ui->cbxProfile->setCurrentText(profileName);

    return profile;
}

void MainWindow::onActionSelChange(int index)
{
    bool enabled = index >= 0;
    if (!enabled) ui->btnActionCreate->setFocus();
    ui->btnActionDelete->setEnabled(enabled);
    ui->btnActionDuplicate->setEnabled(enabled);

    ui->btnActionMoveUp->setEnabled(enabled);
    ui->btnActionMoveDown->setEnabled(enabled);
}

void MainWindow::onActionDoubleClicked(const QModelIndex & index)
{
    Profile * profile = m_Profiles[ui->cbxProfile->currentIndex()];
    Action *  action  = profile->getActionManager()[index.row()];

    m_ActionEditWindow = new ActionEditWindow(this, action);
    connect(m_ActionEditWindow, &ActionEditWindow::onSaved, this, &MainWindow::onActionSaved);
    m_ActionEditWindow->exec();
}

void MainWindow::onBtnActionCreate()
{
    m_ActionEditWindow = new ActionEditWindow(this);
    connect(m_ActionEditWindow, &ActionEditWindow::onCreated, this,
            &MainWindow::onActionCreated);
    m_ActionEditWindow->exec();
}

void MainWindow::onBtnActionDelete()
{
    const int currIndex = ui->lbxActions->currentRow();
    Profile * profile   = m_Profiles[ui->cbxProfile->currentIndex()];
    profile->getActionManager().remove(currIndex);

    saveProfiles();
    reloadProfiles();
    ui->lbxActions->setCurrentRow(currIndex - 1);
}

void MainWindow::onBtnActionDuplicate()
{
    const int currIndex = ui->lbxActions->currentRow();
    Profile * profile   = m_Profiles[ui->cbxProfile->currentIndex()];
    Action *  action    = profile->getActionManager().getByIndex(currIndex);

    Action * newAction =
        new Action(action->getName(), action->getType(), action->getSrcKey(),
                   action->getDstKey(), action->getAppPath(), action->getAppArgs());

    profile->getActionManager().insert(currIndex, newAction);

    saveProfiles();
    reloadProfiles();
    ui->lbxActions->setCurrentRow(currIndex + 1);
}

void MainWindow::onBtnActionMoveDown()
{
    Profile * profile = m_Profiles[ui->cbxProfile->currentIndex()];
    profile->getActionManager().moveDown(ui->lbxActions->currentRow());

    listItemSwap(ui->lbxActions, false);
    saveProfiles();
}

void MainWindow::onBtnActionMoveUp()
{
    Profile * profile = m_Profiles[ui->cbxProfile->currentIndex()];
    profile->getActionManager().moveUp(ui->lbxActions->currentRow());

    listItemSwap(ui->lbxActions, true);
    saveProfiles();
}

void MainWindow::listItemSwap(QListWidget * list, bool moveUp)
{
    const int currIndex = list->currentRow();
    if (currIndex == -1) return;

    if (moveUp)
    {
        if (currIndex > 0)
        {
            QListWidgetItem * item = list->takeItem(currIndex);
            list->insertItem(currIndex - 1, item);
            list->setCurrentRow(currIndex - 1);
        }
    }
    else
    {
        if (currIndex < list->count() - 1)
        {
            QListWidgetItem * item = list->takeItem(currIndex);
            list->insertItem(currIndex + 1, item);
            list->setCurrentRow(currIndex + 1);
        }
    }
}

void MainWindow::onActionSaved()
{
    const int currIndex = ui->lbxActions->currentRow();
    saveProfiles();
    reloadProfiles();
    ui->lbxActions->setCurrentRow(currIndex);
}

void MainWindow::onActionCreated(Action * action)
{
    const int currIndex = ui->lbxActions->currentRow();
    Profile * profile   = m_Profiles[ui->cbxProfile->currentIndex()];
    profile->getActionManager().add(action);
    saveProfiles();
    reloadProfiles();
    ui->lbxActions->setCurrentRow((currIndex >= 0) ? (currIndex + 1) : 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::onActionFileOpen()
{
    QString desktopDir =
        QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    QString srcFilePath = QFileDialog::getOpenFileName(
        this, tr("Open Profiles File"), desktopDir, tr("Profiles File (*.json)"));
    if (srcFilePath.isEmpty()) return;

    auto answer = QMessageBox::warning(
        this, "WARNING",
        "Opening existing profiles file will override the existing profiles\n"
        "Are you sure you want to continue?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (answer != QMessageBox::Yes) return;

    QString dstFilePath = m_Profiles.getConfigFilePath();
    if (!QFile::copy(srcFilePath, dstFilePath))
    {
        qDebug() << "[MainWindow::onActionFileOpen] - Failed to copy file from '"
                 << srcFilePath << "' to '" << dstFilePath << "'";
        return;
    }
}

void MainWindow::onActionFileSave()
{
    m_Profiles.save();
    m_Preference.save();
}

void MainWindow::onActionFileSaveAs()
{
    QString dstFilePath = QFileDialog::getSaveFileName(this, tr("Save As File"), "",
                                                       tr("Profiles File (*.json)"));
    if (dstFilePath.isEmpty()) return;

    QString srcFilePath = m_Profiles.getConfigFilePath();
    if (!QFile::copy(srcFilePath, dstFilePath))
    {
        qDebug() << "[MainWindow::onActionFileSaveAs] - Failed to copy file from '"
                 << srcFilePath << "' to '" << dstFilePath << "'";
        return;
    }
}

void MainWindow::onActionFileRestartService()
{
    QtServiceController service("QuickCut Service");
    service.stop();
    QTimer::singleShot(1000, [&] { activateHook(); });
}

void MainWindow::onActionFileExit()
{
    close();
}

void MainWindow::onActionViewToolBar()
{
    bool visible = ui->actionViewToolBar->isChecked();
    ui->toolBar->setVisible(visible);
    m_Preference.get().setToolBarVisible(visible);
    m_Preference.save();
}

void MainWindow::onActionViewStatusBar()
{
    bool visible = ui->actionViewStatusBar->isChecked();
    ui->statusBar->setVisible(visible);
    m_Preference.get().setStatusBarVisible(visible);
    m_Preference.save();
}

void MainWindow::onActionViewRefresh()
{
    Hook::sendReloadSignal();
    initProfiles();
}

void MainWindow::onActionHelpAbout()
{
    m_AboutWindow = new AboutWindow(this);
    m_AboutWindow->exec();
}

void MainWindow::onActionHelpExamples()
{
    m_ExamplesWindow = new ExamplesWindow(this);
    m_ExamplesWindow->exec();
}

void MainWindow::onActionHelpCheckUpdates()
{
    m_CheckUpdatesWindow = new CheckUpdatesWindow(this);
    m_CheckUpdatesWindow->exec();
}

void MainWindow::onActionLoadTheme(ThemeType type, QAction * action /*= nullptr*/)
{
    onActionLoadTheme(Preference::getThemeResourcePath(type), action);
    m_Preference.get().setCurrentTheme(type);
    m_Preference.save();
}
void MainWindow::onActionLoadTheme(const QString & qssPath, QAction * action /*= nullptr*/)
{
    for (auto && qAction : m_ThemeActions) qAction->setChecked(false);

    if (action) action->setChecked(true);

    QFile file(qssPath);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        qApp->setStyleSheet("");
        return;
    }

    QTextStream ts(&file);
    qApp->setStyleSheet(ts.readAll());
}

void MainWindow::onLoadCustomStylesheet()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Stylesheet File"), "",
                                                    tr("Stylesheet File (*.qss)"));
    if (filePath.isEmpty()) return;

    onActionLoadTheme(filePath);
}
