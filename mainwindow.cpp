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

MainWindow::MainWindow(SingleApplication *parentApp, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainWindow),
    app(parentApp)
{
    // 关机时接收退出信号，释放socket
    QObject::connect(app, &SingleApplication::aboutToQuit, this, &MainWindow::QuitDrcom);

    qDebug()<<"MainWindow constructor";
	ui->setupUi(this);

	CURR_STATE = STATE_OFFLINE;

	// 记住窗口大小功能
	QSettings settings;
	restoreGeometry(settings.value("mainWindowGeometry").toByteArray());

	// 获取mac地址
	foreach(QNetworkInterface i, QNetworkInterface::allInterfaces()) {
		if (!i.flags().testFlag(QNetworkInterface::IsLoopBack)) {
			ui->comboBoxMAC->addItem(i.hardwareAddress() + i.name());
		}
	}
	ui->comboBoxMAC->addItem(CUSTOM_MAC);

	// 重启功能
	restartAction = new QAction(tr("Re&start"), this);
	connect(restartAction, &QAction::triggered, this, &MainWindow::RestartDrcomByUser);

	// 创建托盘菜单和图标
	// 托盘菜单选项
	restoreAction = new QAction(tr("&Restore"), this);
	connect(restoreAction, &QAction::triggered, this, &MainWindow::ShowLoginWindow);
	logOutAction = new QAction(tr("&Logout"), this);
	connect(logOutAction, &QAction::triggered, this, &MainWindow::UserLogOut);
	quitAction = new QAction(tr("&Quit"), this);
	connect(quitAction, &QAction::triggered, this, &MainWindow::QuitDrcom);
	// 新建菜单
	trayIconMenu = new QMenu(this);
	trayIconMenu->addAction(restoreAction);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(logOutAction);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(restartAction);
	trayIconMenu->addAction(quitAction);
	// 新建托盘图标
	trayIcon = new QSystemTrayIcon(this);
	trayIcon->setContextMenu(trayIconMenu);
	// 设置托盘菜单响应函数
	connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::IconActivated);
	// 设置托盘图标和窗口图标
	SetIcon(false);
	// 显示出来托盘图标
	trayIcon->show();

	// 创建窗口菜单
	aboutAction = new QAction(tr("&About"), this);
	connect(aboutAction, &QAction::triggered, this, &MainWindow::AboutDrcom);
	windowMenu = new QMenu(tr("&Help"), this);
	windowMenu->addAction(aboutAction);
	windowMenu->addAction(logOutAction);
	ui->menuBar->addMenu(windowMenu);
	// 重启是个专门的菜单按钮
	ui->menuBar->addAction(restartAction);

	// 读取配置文件
	LoadSettings();

	// 设置回调函数
	dogcomController = new DogcomController();
	connect(dogcomController, &DogcomController::HaveBeenOffline,
		this, &MainWindow::HandleOffline);
	connect(dogcomController, &DogcomController::HaveLoggedIn,
		this, &MainWindow::HandleLoggedIn);
	connect(dogcomController, &DogcomController::HaveObtainedIp,
		this, &MainWindow::HandleIpAddress);

	// 验证手动输入的mac地址
	macValidator = new QRegExpValidator(QRegExp("[0-9a-fA-F]{2}(:[0-9a-fA-F]{2}){5}"));
	ui->lineEditMAC->setValidator(macValidator);

	// 尚未登录 不可注销
	DisableLogOutButton(true);

	// 自动登录功能
	QSettings s(SETTINGS_FILE_NAME);
	int restartTimes = s.value(ID_RESTART_TIMES, 0).toInt();
	qDebug() << "MainWindow constructor: restartTimes=" << restartTimes;
	if (restartTimes == 0) {
        if (bAutoLogin) {
			emit ui->pushButtonLogin->click();
		}
	}
	else {
		// 尝试自动重启中
		emit ui->pushButtonLogin->click();
	}
}

void MainWindow::AboutDrcom() {
	QDesktopServices::openUrl(QUrl("https://github.com/code4lala/drcom-jlu-qt"));
}

void MainWindow::closeEvent(QCloseEvent *) {
	QSettings settings;
	settings.setValue("mainWindowGeometry", saveGeometry());
	// 未登录时直接关闭窗口就退出
	if (CURR_STATE == STATE_OFFLINE) {
		QuitDrcom();
	}
}

void MainWindow::ShowLoginWindow() {
	if (!isVisible()) {
		// 若登录窗口没显示则显示出来
		showNormal();
	}
	else {
		// 已显示则将窗口设为焦点
		activateWindow();
	}
}

void MainWindow::RestartDrcom()
{
    bRestart=true;
    if(CURR_STATE==STATE_ONLINE)
        dogcomController->LogOut();
    else if(CURR_STATE==STATE_OFFLINE){
        qDebug() << "quiting current instance...";
        qApp->quit();
        qDebug() << "Restarting Drcom...";
        QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
        qDebug() << "Restart done.";
    }
    else if(CURR_STATE==STATE_LOGGING)
        ;// 正在登录时候退出，假装没看到，不理
}

void MainWindow::QuitDrcom()
{
	// 退出之前恢复重试计数
	QSettings s(SETTINGS_FILE_NAME);
	s.setValue(ID_RESTART_TIMES, 0);
	qDebug() << "reset restartTimes";
	qDebug() << "QuitDrcom";

    // TODO : release socket
    bQuit=true;
    if(CURR_STATE==STATE_ONLINE)
        dogcomController->LogOut();
    else if(CURR_STATE==STATE_OFFLINE)
        qApp->quit();
    else if(CURR_STATE==STATE_LOGGING)
        ;// 正在登录时候退出，假装没看到，不理

    // qApp->quit()调用放到了注销响应那块
}

void MainWindow::RestartDrcomByUser()
{
	// 重启之前恢复重试计数
	QSettings s(SETTINGS_FILE_NAME);
	s.setValue(ID_RESTART_TIMES, 0);
	qDebug() << "reset restartTimes";
	RestartDrcom();
}

void MainWindow::IconActivated(QSystemTrayIcon::ActivationReason reason)
{
	switch (reason) {
	case QSystemTrayIcon::DoubleClick: {
		restoreAction->activate(restoreAction->Trigger);
		break;
	}
	default:
		break;
	}
}

void MainWindow::LoadSettings() {
	QSettings s(SETTINGS_FILE_NAME);
	account = s.value(ID_ACCOUNT, "").toString();
	password = s.value(ID_PASSWORD, "").toString();
	mac_addr = s.value(ID_MAC, "").toString();
    bRemember = s.value(ID_REMEMBER, false).toBool();
    bAutoLogin = s.value(ID_AUTO_LOGIN, false).toBool();
    bHideWindow = s.value(ID_HIDE_WINDOW, false).toBool();
    bNotShowWelcome = s.value(ID_NOT_SHOW_WELCOME, false).toBool();
	ui->lineEditAccount->setText(account);
	ui->lineEditPass->setText(password);
	SetMAC(mac_addr);
    if (bRemember)
		ui->checkBoxRemember->setCheckState(Qt::CheckState::Checked);
	else
		ui->checkBoxRemember->setCheckState(Qt::CheckState::Unchecked);
    if (bAutoLogin)
		ui->checkBoxAutoLogin->setCheckState(Qt::CheckState::Checked);
	else
		ui->checkBoxAutoLogin->setCheckState(Qt::CheckState::Unchecked);
    if(bHideWindow)
        ui->checkBoxHideLoginWindow->setCheckState(Qt::CheckState::Checked);
    else
        ui->checkBoxHideLoginWindow->setCheckState(Qt::CheckState::Unchecked);
    if(bNotShowWelcome)
        ui->checkBoxNotShowWelcome->setCheckState(Qt::CheckState::Checked);
    else
        ui->checkBoxNotShowWelcome->setCheckState(Qt::CheckState::Unchecked);
}

void MainWindow::SaveSettings() {
	QSettings s(SETTINGS_FILE_NAME);
	s.setValue(ID_ACCOUNT, account);
	s.setValue(ID_PASSWORD, password);
	s.setValue(ID_MAC, mac_addr);
    s.setValue(ID_REMEMBER, bRemember);
    s.setValue(ID_AUTO_LOGIN, bAutoLogin);
}

void MainWindow::SetMAC(const QString &m)
{
	for (int i = 0; i < ui->comboBoxMAC->count(); i++) {
		QString s = ui->comboBoxMAC->itemText(i);
		if (!s.compare(CUSTOM_MAC))continue;
		if (s.indexOf(m) != -1) {
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
    qDebug()<<"MainWindow destructor";
	delete ui;
    delete restartAction;
    delete restoreAction;
    delete logOutAction;
    delete quitAction;
    delete trayIconMenu;
    delete trayIcon;
    delete aboutAction;
    delete windowMenu;
    delete dogcomController;
    delete macValidator;
}

void MainWindow::on_checkBoxAutoLogin_toggled(bool checked)
{
	if (checked) {
		ui->checkBoxRemember->setChecked(true);
	}
}

void MainWindow::on_checkBoxRemember_toggled(bool checked)
{
	if (!checked) {
		ui->checkBoxAutoLogin->setChecked(false);
	}
}

void MainWindow::on_comboBoxMAC_currentTextChanged(const QString &arg1)
{
	if (!arg1.compare(CUSTOM_MAC)) {
		ui->lineEditMAC->setDisabled(false);
	}
	else {
		ui->lineEditMAC->setDisabled(true);
	}
}

void MainWindow::on_pushButtonLogin_clicked()
{
	GetInputs();
	if (!account.compare("")
		|| !password.compare("")
        || !mac_addr.compare("")) {
		QMessageBox::warning(this, APP_NAME, tr("Input can not be empty!"));
		return;
	}
	if (mac_addr.length() != 17) {
		QMessageBox::warning(this, APP_NAME, tr("Illegal MAC address!"));
		return;
	}
	// 输入无误，执行登录操作
	// 先禁用输入框和按钮
	SetDisableInput(true);
	CURR_STATE = STATE_LOGGING;
	dogcomController->Login(account, password, mac_addr);
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
	if (!yes) {
		// 要启用输入框
		if (ui->comboBoxMAC->currentText().compare(CUSTOM_MAC)) {
			// 当前选的不是自定义mac地址
			ui->lineEditMAC->setDisabled(true);
		}
	}
}

void MainWindow::SetIcon(bool online)
{
	QIcon icon;
	QString toolTip;
	if (online) {
		// 设置彩色图标
		icon = QIcon(":/images/online.png");
		toolTip = tr("DrCOM JLU Qt -- online");
	}
	else {
		// 设置灰色图标
		icon = QIcon(":/images/offline.png");
		toolTip = tr("DrCOM JLU Qt -- offline");
	}
	trayIcon->setIcon(icon);
	trayIcon->setToolTip(toolTip);
	setWindowIcon(icon);
}

void MainWindow::GetInputs() {
	account = ui->lineEditAccount->text();
	password = ui->lineEditPass->text();
	if (ui->lineEditMAC->isEnabled()) {
		mac_addr = ui->lineEditMAC->text();
	}
	else {
		mac_addr = ui->comboBoxMAC->currentText();
	}
	mac_addr = mac_addr.remove(17, mac_addr.length()).toUpper();
    bRemember = ui->checkBoxRemember->checkState();
    bAutoLogin = ui->checkBoxAutoLogin->checkState();
}

void MainWindow::HandleOffline(int reason)
{
	CURR_STATE = STATE_OFFLINE;
	ui->pushButtonLogin->setText(tr("Login"));
	switch (reason) {
	case OFF_USER_LOGOUT: {
        if(bQuit){
            qApp->quit();
            return;
        }
        if(bRestart){
            qApp->quit();
            qDebug() << "Restarting Drcom...";
            QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
            qDebug() << "Restart done.";
            return;
        }
        QMessageBox::information(this, tr("Logout succeed"), tr("Logout succeed"));
		break;
	}
	case OFF_BIND_FAILED: {
		QMessageBox::critical(this, tr("Login failed"), tr("Binding port failed. Please check if there are other clients occupying the port"));
		break;
	}
	case OFF_CHALLENGE_FAILED: {
		// 弹出一个提示框，带一个直接重启客户端的按钮
		QMessageBox msgBox;
		msgBox.setText(tr("Login failed") + " " + tr("Challenge failed. Please check your connection:)") + " " +
			tr("Attention that you should connect to wifi or wired firstly and then start the drcom client. If you have connected, you may restart drcom to solve the problem.")
			+ " " + tr("Restart DrCOM?"));
		QAbstractButton *pButtonYes = msgBox.addButton(tr("Yes"), QMessageBox::YesRole);
		msgBox.addButton(tr("Nope"), QMessageBox::NoRole);
		msgBox.exec();
		if (msgBox.clickedButton() == pButtonYes) {
			qDebug() << "Restart DrCOM confirmed in case OFF_CHALLENGE_FAILED";
			RestartDrcomByUser();
		}
		break;
	}
	case OFF_CHECK_MAC: {
		QMessageBox::critical(this, tr("Login failed"), tr("Someone is using this account with wired"));
		break;
	}
	case OFF_SERVER_BUSY: {
		QMessageBox::critical(this, tr("Login failed"), tr("The server is busy, please log back in again"));
		break;
	}
	case OFF_WRONG_PASS: {
		QMessageBox::critical(this, tr("Login failed"), tr("Account and password not match"));
		break;
	}
	case OFF_NOT_ENOUGH: {
		QMessageBox::critical(this, tr("Login failed"), tr("The cumulative time or traffic for this account has exceeded the limit"));
		break;
	}
	case OFF_FREEZE_UP: {
		QMessageBox::critical(this, tr("Login failed"), tr("This account is suspended"));
		break;
	}
	case OFF_NOT_ON_THIS_IP: {
		QMessageBox::critical(this, tr("Login failed"), tr("IP address does not match, this account can only be used in the specified IP address"));
		break;
	}
	case OFF_NOT_ON_THIS_MAC: {
		QMessageBox::critical(this, tr("Login failed"), tr("MAC address does not match, this account can only be used in the specified IP and MAC address"));
		break;
	}
	case OFF_TOO_MUCH_IP: {
		QMessageBox::critical(this, tr("Login failed"), tr("This account has too many IP addresses"));
		break;
	}
	case OFF_UPDATE_CLIENT: {
		QMessageBox::critical(this, tr("Login failed"), tr("The client version is incorrect"));
		break;
	}
	case OFF_NOT_ON_THIS_IP_MAC: {
		QMessageBox::critical(this, tr("Login failed"), tr("This account can only be used on specified MAC and IP address"));
		break;
	}
	case OFF_MUST_USE_DHCP: {
		QMessageBox::critical(this, tr("Login failed"), tr("Your PC set up a static IP, please change to DHCP, and then re-login"));
		break;
	}
	case OFF_TIMEOUT: {
		// 先尝试自己重启若干次，自个重启还不行的话再提示用户
		// 自己重启的话需要用户提前勾选记住密码
        if (bRemember) {
			QSettings s(SETTINGS_FILE_NAME);
			int restartTimes = s.value(ID_RESTART_TIMES, 0).toInt();
			qDebug() << "case OFF_TIMEOUT: restartTimes=" << restartTimes;
			if (restartTimes <= RETRY_TIMES) {
				qDebug() << "case OFF_TIMEOUT: restartTimes++";
				s.setValue(ID_RESTART_TIMES, restartTimes + 1);
				//尝试自行重启
				qDebug() << "trying to restart to reconnect...";
				qDebug() << "restarting for the" << restartTimes << "times";
				RestartDrcom();
				return;
			}
		}

		// 弹出一个提示框，带一个直接重启客户端的按钮
		QMessageBox msgBox;
		msgBox.setText(tr("You have been offline") + " " + tr("Time out, please check your connection")
			+ " " + tr("The DrCOM client will try to restart to solve some unstable problems but the function relies on \"remember me\"")
			+ " " + tr("Due to some reasons, you should connect to wifi or wired firstly and then start the drcom client. So you may not login until you restart DrCOM :D")
			+ " " + tr("Restart DrCOM?"));
		QAbstractButton *pButtonYes = msgBox.addButton(tr("Yes"), QMessageBox::YesRole);
		msgBox.addButton(tr("Nope"), QMessageBox::NoRole);
		msgBox.exec();
		if (msgBox.clickedButton() == pButtonYes) {
			qDebug() << "Restart DrCOM confirmed in case OFF_TIMEOUT";
			RestartDrcomByUser();
		}
		break;
	}
	case OFF_UNKNOWN:
	default:
		QMessageBox::critical(this, tr("You have been offline"), tr("Unknow reason"));
		break;
	}
	if (reason == OFF_WRONG_PASS) {
		// 清除已保存的密码
		account = "";
		password = "";
        bRemember = false;
        bAutoLogin = false;
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
	QSettings s(SETTINGS_FILE_NAME);
	int restartTimes = s.value(ID_RESTART_TIMES, 0).toInt();
	qDebug() << "HandleLoggedIn: restartTimes=" << restartTimes;

	CURR_STATE = STATE_ONLINE;
	// 显示欢迎页
    bool bNotShowWelcome = s.value(ID_NOT_SHOW_WELCOME, false).toBool();
    if (restartTimes == 0 && !bNotShowWelcome) {
        qDebug()<<"open welcome page";
		QDesktopServices::openUrl(QUrl("http://login.jlu.edu.cn/notice.php"));
	}
	else {
		// 自行尝试重启的，不显示欢迎页
	}
	// 登录成功，保存密码
    if (bRemember) {
		SaveSettings();
	}
	else {
		account = "";
		password = "";
		SaveSettings();
	}
	SetIcon(true);
	// 启用注销按钮
	DisableLogOutButton(false);

	s.setValue(ID_RESTART_TIMES, 0);
	qDebug() << "HandleLoggedIn: reset restartTimes";
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

void MainWindow::DisableLogOutButton(bool yes) {
	if (yes) {
		logOutAction->setDisabled(true);
	}
	else {
		logOutAction->setEnabled(true);
	}
}

void MainWindow::on_checkBoxNotShowWelcome_toggled(bool checked)
{
    QSettings s(SETTINGS_FILE_NAME);
    s.setValue(ID_NOT_SHOW_WELCOME, checked);
}

void MainWindow::on_checkBoxHideLoginWindow_toggled(bool checked)
{
    QSettings s(SETTINGS_FILE_NAME);
    s.setValue(ID_HIDE_WINDOW, checked);
}
