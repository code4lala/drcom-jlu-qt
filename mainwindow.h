#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDialog>
#include <QSettings>
#include <QRegExpValidator>
#include <dogcomcontroller.h>
#include <QSystemTrayIcon>

namespace Ui {
class MainWindow;
}

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_checkBoxAutoLogin_toggled(bool checked);

    void on_checkBoxRemember_toggled(bool checked);

    void on_comboBoxMAC_currentTextChanged(const QString &arg1);

    void on_pushButtonLogin_clicked();

    void IconActivated(QSystemTrayIcon::ActivationReason reason);

    void UserLogOut();

public slots:
    void HandleOffline(int reason);
    void HandleLoggedIn();
    void HandleIpAddress(const QString &ip);
    void ShowLoginWindow();
    void RestartDrcom();

private:
    Ui::MainWindow *ui;
    QString SETTINGS_FILE_NAME="DrCOM_JLU_Qt.ini";
    const QString CUSTOM_MAC=tr("custom (format: 1A:2B:3C:4D:5E:6F case insensitive)");
    const QString APP_NAME=tr("DrCOM JLU Qt version");
    const QString
        idAccount="account",
        idPassword="password",
        idMac="mac",
        idRemember="remember",
        idAutoLogin="autoLogin";

    // 记录用户保存的信息
    QString account,password,mac_addr;
    bool remember,auto_login;

    // 用于在未登录时关闭窗口就退出
    int CURR_STATE;

    QRegExpValidator *macValidator;
    DogcomController *dogcomController;

    // 设置托盘中的注销按钮的可用性
    void DisableLogOutButton(bool yes);

    // 窗口菜单
    QAction *aboutAction;
    QMenu *windowMenu;
    void AboutDrcom();

    // 托盘图标
    QAction *restartAction;
    QAction *restoreAction;
    QAction *logOutAction;
    QAction *quitAction;
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    void CreateActions();
    void CreateTrayIcon();
    void SetIcon(bool online);

    void GetInputs();
    void LoadSettings();
    void SaveSettings();
    void SetMAC(const QString &m);
    void SetDisableInput(bool yes);
};

#endif // MAINWINDOW_H
