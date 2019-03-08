﻿#include "mainwindow.h"
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

MainWindow::MainWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 获取mac地址
    foreach(QNetworkInterface i, QNetworkInterface::allInterfaces()){
        if(!i.flags().testFlag(QNetworkInterface::IsLoopBack)){
//            qDebug()<<i<<endl;
            ui->comboBoxMAC->addItem(i.hardwareAddress()+i.name());
        }
    }
    ui->comboBoxMAC->addItem(CUSTOM_MAC);

    // 创建托盘菜单和图标
    // 托盘菜单选项
    restoreAction=new QAction(tr("&Restore"),this);
    connect(restoreAction,&QAction::triggered,this,&MainWindow::ShowLoginWindow);
    logOutAction=new QAction(tr("&Logout"),this);
    connect(logOutAction,&QAction::triggered,this,&MainWindow::UserLogOut);
    quitAction=new QAction(tr("&Quit"),this);
    connect(quitAction,&QAction::triggered,qApp,&QCoreApplication::quit);
    // 新建菜单
    trayIconMenu=new QMenu(this);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(logOutAction);
    trayIconMenu->addSeparator();
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

    // 读取配置文件
    LoadSettings();

    // 设置回调函数
    dogcomController=new DogcomController();
    connect(dogcomController, &DogcomController::HaveBeenOffline,
            this, &MainWindow::HandleOffline);
    connect(dogcomController, &DogcomController::HaveLoggedIn,
            this, &MainWindow::HandleLoggedIn);

    // 验证手动输入的mac地址
    macValidator=new QRegExpValidator(QRegExp("[0-9a-fA-F]{2}(:[0-9a-fA-F]{2}){5}"));
    ui->lineEditMAC->setValidator(macValidator);

    // 尚未登录 不可注销
    DisableLogOutButton(true);

    // 自动登录功能
    if(auto_login){
        ui->pushButtonLogin->click();
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
        QMessageBox::warning(this, APP_NAME, "Input can not be empty!");
        return;
    }
    if(mac_addr.length()!=17){
        QMessageBox::warning(this, APP_NAME, "Illegal MAC address!");
        return;
    }
    // 输入无误，执行登录操作
    // 先禁用输入框和按钮
    SetDisableInput(true);
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
//    qDebug()<<"account:"<<account;
//    qDebug()<<"password:"<<password;
//    qDebug()<<"mac_addr:"<<mac_addr;
//    qDebug()<<"remember:"<<remember;
//    qDebug()<<"auto_login:"<<auto_login;
//    qDebug()<<endl;
}

void MainWindow::HandleOffline(int reason)
{
    switch (reason) {
    case OFF_UNKNOWN:{
        break;
    }
    case OFF_WRONG_PASS:{
        QMessageBox::critical(this, tr("Login failed"), tr("Wrong password!"));
        break;
    }
    case OFF_WRONG_MAC:{
        break;
    }
    default:break;
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
}

void MainWindow::HandleLoggedIn()
{
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
