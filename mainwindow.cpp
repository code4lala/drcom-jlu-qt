#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtNetwork/QNetworkInterface>
#include <QList>
#include <QComboBox>
#include <QDebug>
#include <QMessageBox>
#include <QValidator>
#include <QRegExp>
#include <QSettings>
#include <QWindow>
#include "constants.h"
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QCloseEvent>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    CURR_STATE=STATE_OFFLINE;

    // 记住窗口大小功能
    QSettings settings;
    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());

    // 获取mac地址
    foreach(QNetworkInterface i, QNetworkInterface::allInterfaces()){
        if(!i.flags().testFlag(QNetworkInterface::IsLoopBack)){
            ui->comboBoxMAC->addItem(i.hardwareAddress()+i.name());
        }
    }
    ui->comboBoxMAC->addItem(CUSTOM_MAC);

    // 重启功能
    restartAction=new QAction(tr("Re&start"),this);
    connect(restartAction,&QAction::triggered,this,&MainWindow::RestartDrcom);

    // 创建托盘菜单和图标
    // 托盘菜单选项
    restoreAction=new QAction(tr("&Restore"),this);
    connect(restoreAction,&QAction::triggered,this,&MainWindow::ShowLoginWindow);
    logOutAction=new QAction(tr("&Logout"),this);
    connect(logOutAction,&QAction::triggered,this,&MainWindow::UserLogOut);
    quitAction=new QAction(tr("&Quit"),this);
    connect(quitAction,&QAction::triggered,qApp,&QApplication::quit);
    // 新建菜单
    trayIconMenu=new QMenu(this);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(logOutAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(restartAction);
    trayIconMenu->addAction(quitAction);
    // 新建托盘图标
    trayIcon=new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    // 设置托盘菜单响应函数
    connect(trayIcon,&QSystemTrayIcon::activated,this,&MainWindow::IconActivated);
    // 设置托盘图标和窗口图标
    SetIcon(false);
    // 显示出来托盘图标
    trayIcon->show();

    // 创建窗口菜单
    aboutAction=new QAction(tr("&About"),this);
    connect(aboutAction,&QAction::triggered,this,&MainWindow::AboutDrcom);
    windowMenu=new QMenu(tr("&Help"),this);
    windowMenu->addAction(aboutAction);
    windowMenu->addAction(logOutAction);
    ui->menuBar->addMenu(windowMenu);
    // 重启是个专门的菜单按钮
    ui->menuBar->addAction(restartAction);

    // 读取配置文件
    LoadSettings();

    // 设置回调函数
    dogcomController=new DogcomController();
    connect(dogcomController, &DogcomController::HaveBeenOffline,
            this, &MainWindow::HandleOffline);
    connect(dogcomController, &DogcomController::HaveLoggedIn,
            this, &MainWindow::HandleLoggedIn);
    connect(dogcomController,&DogcomController::HaveObtainedIp,
            this,&MainWindow::HandleIpAddress);

    // 验证手动输入的mac地址
    macValidator=new QRegExpValidator(QRegExp("[0-9a-fA-F]{2}(:[0-9a-fA-F]{2}){5}"));
    ui->lineEditMAC->setValidator(macValidator);

    // 尚未登录 不可注销
    DisableLogOutButton(true);

    // 自动登录功能
    if(auto_login){
        emit ui->pushButtonLogin->click();
    }
}

void MainWindow::AboutDrcom(){
    QDesktopServices::openUrl(QUrl("https://github.com/code4lala/drcom-jlu-qt"));
}

void MainWindow::closeEvent(QCloseEvent *event){
  QSettings settings;
  settings.setValue("mainWindowGeometry", saveGeometry());
  // 未登录时直接关闭窗口就退出
  if(CURR_STATE==STATE_OFFLINE){
      QApplication::quit();
  }
}

void MainWindow::ShowLoginWindow(){
    if(!isVisible()){
    // 若登录窗口没显示则显示出来
        showNormal();
    }else{
    // 已显示则将窗口设为焦点
        activateWindow();
    }
}

void MainWindow::RestartDrcom()
{
    qDebug()<<"Restarting Drcom...";
    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0],qApp->arguments());
}

void MainWindow::IconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::DoubleClick:{
        restoreAction->activate(restoreAction->Trigger);
        break;
    }
    default:
        break;
    }
}

void MainWindow::LoadSettings(){
    QSettings s(SETTINGS_FILE_NAME);
    account=s.value(idAccount,"").toString();
    password=s.value(idPassword,"").toString();
    mac_addr=s.value(idMac,"").toString();
    remember=s.value(idRemember,false).toBool();
    auto_login=s.value(idAutoLogin,false).toBool();
    ui->lineEditAccount->setText(account);
    ui->lineEditPass->setText(password);
    SetMAC(mac_addr);
    if(remember)
        ui->checkBoxRemember->setCheckState(Qt::CheckState::Checked);
    else
        ui->checkBoxRemember->setCheckState(Qt::CheckState::Unchecked);
    if(auto_login)
        ui->checkBoxAutoLogin->setCheckState(Qt::CheckState::Checked);
    else
        ui->checkBoxAutoLogin->setCheckState(Qt::CheckState::Unchecked);
}

void MainWindow::SaveSettings(){
    QSettings s(SETTINGS_FILE_NAME);
    s.setValue(idAccount,account);
    s.setValue(idPassword,password);
    s.setValue(idMac,mac_addr);
    s.setValue(idRemember,remember);
    s.setValue(idAutoLogin,auto_login);
}

void MainWindow::SetMAC(const QString &m)
{
    for(int i=0;i<ui->comboBoxMAC->count();i++){
        QString s=ui->comboBoxMAC->itemText(i);
        if(!s.compare(CUSTOM_MAC))continue;
        if(s.indexOf(m)!=-1){
            //当前列表中有该mac地址
            ui->comboBoxMAC->setCurrentIndex(i);
            return;
        }
    }
    // 当前列表中没有该mac地址，填充到输入框中
    ui->comboBoxMAC->setCurrentText(CUSTOM_MAC);
    ui->lineEditMAC->setText(m);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_checkBoxAutoLogin_toggled(bool checked)
{
    if(checked){
        ui->checkBoxRemember->setChecked(true);
    }
}

void MainWindow::on_checkBoxRemember_toggled(bool checked)
{
    if(!checked){
        ui->checkBoxAutoLogin->setChecked(false);
    }
}

void MainWindow::on_comboBoxMAC_currentTextChanged(const QString &arg1)
{
    if(!arg1.compare(CUSTOM_MAC)){
        ui->lineEditMAC->setDisabled(false);
    }else{
        ui->lineEditMAC->setDisabled(true);
    }
}

void MainWindow::on_pushButtonLogin_clicked()
{
    GetInputs();
    if(!account.compare("")
     ||!password.compare("")
     ||!mac_addr.compare("")){
        QMessageBox::warning(this, APP_NAME, tr("Input can not be empty!"));
        return;
    }
    if(mac_addr.length()!=17){
        QMessageBox::warning(this, APP_NAME, tr("Illegal MAC address!"));
        return;
    }
    // 输入无误，执行登录操作
    // 先禁用输入框和按钮
    SetDisableInput(true);
    CURR_STATE=STATE_LOGGING;
    dogcomController->Login(account,password,mac_addr);
}

void MainWindow::SetDisableInput(bool yes)
{
    ui->lineEditAccount->setDisabled(yes);
    ui->lineEditPass->setDisabled(yes);
    ui->comboBoxMAC->setDisabled(yes);
    ui->lineEditMAC->setDisabled(yes);
    ui->checkBoxRemember->setDisabled(yes);
    ui->checkBoxAutoLogin->setDisabled(yes);
    ui->pushButtonLogin->setDisabled(yes);
    if(!yes){
        // 要启用输入框
        if(ui->comboBoxMAC->currentText().compare(CUSTOM_MAC)){
            // 当前选的不是自定义mac地址
            ui->lineEditMAC->setDisabled(true);
        }
    }
}

void MainWindow::SetIcon(bool online)
{
    QIcon icon;
    QString toolTip;
    if(online){
        // 设置彩色图标
        icon=QIcon(":/images/online.png");
        toolTip=tr("DrCOM JLU Qt -- online");
    }else{
        // 设置灰色图标
        icon=QIcon(":/images/offline.png");
        toolTip=tr("DrCOM JLU Qt -- offline");
    }
    trayIcon->setIcon(icon);
    trayIcon->setToolTip(toolTip);
    setWindowIcon(icon);
}

void MainWindow::GetInputs(){
    account=ui->lineEditAccount->text();
    password=ui->lineEditPass->text();
    if(ui->lineEditMAC->isEnabled()){
        mac_addr=ui->lineEditMAC->text();
    }else{
        mac_addr=ui->comboBoxMAC->currentText();
    }
    mac_addr=mac_addr.remove(17,mac_addr.length()).toUpper();
    remember=ui->checkBoxRemember->checkState();
    auto_login=ui->checkBoxAutoLogin->checkState();
}

void MainWindow::HandleOffline(int reason)
{
    CURR_STATE=STATE_OFFLINE;
    ui->pushButtonLogin->setText(tr("Login"));
    switch (reason) {
    case OFF_USER_LOGOUT:{
        QMessageBox::information(this,tr("Logout succeed"),tr("Logout succeed"));
        break;
    }
    case OFF_BIND_FAILED:{
        QMessageBox::critical(this,tr("Login failed"),tr("Binding port failed. Please check if there are other clients occupying the port"));
        break;
    }
    case OFF_CHALLENGE_FAILED:{
        // 弹出一个提示框，带一个直接重启客户端的按钮
        QMessageBox msgBox;
        msgBox.setText(tr("Login failed")+" "+tr("Challenge failed. Please check your connection:)")+" "+
                       tr("Attention that you should connect to wifi or wired firstly and then start the drcom client. If you have connected, you may restart drcom to solve the problem.")
                       +" "+tr("Restart DrCOM?"));
        QAbstractButton *pButtonYes=msgBox.addButton(tr("Yes"),QMessageBox::YesRole);
        msgBox.addButton(tr("Nope"),QMessageBox::NoRole);
        msgBox.exec();
        if(msgBox.clickedButton()==pButtonYes){
            qDebug()<<"Restart DrCOM confirmed";
            RestartDrcom();
        }
        break;
    }
    case OFF_CHECK_MAC:{
        QMessageBox::critical(this,tr("Login failed"),tr("Someone is using this account with wired"));
        break;
    }
    case OFF_SERVER_BUSY:{
        QMessageBox::critical(this,tr("Login failed"),tr("The server is busy, please log back in again"));
        break;
    }
    case OFF_WRONG_PASS:{
        QMessageBox::critical(this,tr("Login failed"),tr("Account and password not match"));
        break;
    }
    case OFF_NOT_ENOUGH:{
        QMessageBox::critical(this,tr("Login failed"),tr("The cumulative time or traffic for this account has exceeded the limit"));
        break;
    }
    case OFF_FREEZE_UP:{
        QMessageBox::critical(this,tr("Login failed"),tr("This account is suspended"));
        break;
    }
    case OFF_NOT_ON_THIS_IP:{
        QMessageBox::critical(this,tr("Login failed"),tr("IP address does not match, this account can only be used in the specified IP address"));
        break;
    }
    case OFF_NOT_ON_THIS_MAC:{
        QMessageBox::critical(this,tr("Login failed"),tr("MAC address does not match, this account can only be used in the specified IP and MAC address"));
        break;
    }
    case OFF_TOO_MUCH_IP:{
        QMessageBox::critical(this,tr("Login failed"),tr("This account has too many IP addresses"));
        break;
    }
    case OFF_UPDATE_CLIENT:{
        QMessageBox::critical(this,tr("Login failed"),tr("The client version is incorrect"));
        break;
    }
    case OFF_NOT_ON_THIS_IP_MAC:{
        QMessageBox::critical(this,tr("Login failed"),tr("This account can only be used on specified MAC and IP address"));
        break;
    }
    case OFF_MUST_USE_DHCP:{
        QMessageBox::critical(this,tr("Login failed"),tr("Your PC set up a static IP, please change to DHCP, and then re-login"));
        break;
    }
    case OFF_TIMEOUT:{
        // 弹出一个提示框，带一个直接重启客户端的按钮
        QMessageBox msgBox;
        msgBox.setText(tr("You have been offline")+" "+tr("Time out, please check your connection")
                       +" "+tr("Due to some reasons, you should connect to wifi or wired firstly and then start the drcom client. So you may not login until you restart DrCOM :D")
                       +" "+tr("Restart DrCOM?"));
        QAbstractButton *pButtonYes=msgBox.addButton(tr("Yes"),QMessageBox::YesRole);
        msgBox.addButton(tr("Nope"),QMessageBox::NoRole);
        msgBox.exec();
        if(msgBox.clickedButton()==pButtonYes){
            qDebug()<<"Restart DrCOM confirmed";
            RestartDrcom();
        }
        break;
    }
    case OFF_UNKNOWN:
    default:
        QMessageBox::critical(this, tr("You have been offline"), tr("Unknow reason"));
        break;
    }
    if(reason==OFF_WRONG_PASS){
        // 清除已保存的密码
        account="";
        password="";
        remember=false;
        auto_login=false;
        SaveSettings();
    }
    // 重新启用输入
    SetDisableInput(false);
    SetIcon(false);
    // 禁用注销按钮
    DisableLogOutButton(true);
    // 显示出窗口
    ShowLoginWindow();
}

void MainWindow::HandleLoggedIn()
{
    CURR_STATE=STATE_ONLINE;
    // 显示欢迎页
    QDesktopServices::openUrl(QUrl("http://login.jlu.edu.cn/notice.php"));
    // 登录成功，保存密码
    if(remember){
        SaveSettings();
    }else{
        account="";
        password="";
        SaveSettings();
    }
    SetIcon(true);
    // 启用注销按钮
    DisableLogOutButton(false);
}

void MainWindow::HandleIpAddress(const QString &ip)
{
    ui->labelIp->setText(ip);
}

void MainWindow::UserLogOut()
{
    // 用户主动注销
    dogcomController->LogOut();
    // 注销后应该是想重新登录或者换个号，因此显示出用户界面
    restoreAction->activate(restoreAction->Trigger);
    // 禁用注销按钮
    DisableLogOutButton(true);
}

void MainWindow::DisableLogOutButton(bool yes){
    if(yes){
        logOutAction->setDisabled(true);
    }else{
        logOutAction->setEnabled(true);
    }
}
