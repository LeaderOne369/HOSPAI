#include "LoginDialog.h"
#include "ui_LoginDialog.h"
#include "RegisterDialog.h"
#include "ForgotPasswordDialog.h"
#include <QMessageBox>
#include <QKeyEvent>
#include <QTimer>
#include <QRegularExpression>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
    , m_loginSuccess(false)
{
    ui->setupUi(this);
    
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_TranslucentBackground, false);
    
    setupConnections();
    setupAnimations();
    
    // 设置默认焦点
    ui->usernameEdit->setFocus();
    
    // 设置下拉框数据
    ui->roleCombo->setItemData(0, "患者");
    ui->roleCombo->setItemData(1, "客服");
    ui->roleCombo->setItemData(2, "管理员");
    
    // 初始隐藏消息标签
    ui->messageLabel->hide();
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::setupConnections()
{
    // 连接信号和槽
    connect(ui->usernameEdit, &QLineEdit::textChanged, this, &LoginDialog::onUsernameChanged);
    connect(ui->passwordEdit, &QLineEdit::textChanged, this, &LoginDialog::onPasswordChanged);
    connect(ui->passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(ui->registerButton, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);
    connect(ui->forgotButton, &QPushButton::clicked, this, &LoginDialog::onForgotPasswordClicked);
    connect(ui->roleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &LoginDialog::onRoleChanged);
}

void LoginDialog::setupAnimations()
{
    // 阴影效果
    m_shadowEffect = new QGraphicsDropShadowEffect;
    m_shadowEffect->setBlurRadius(20);
    m_shadowEffect->setXOffset(0);
    m_shadowEffect->setYOffset(10);
    m_shadowEffect->setColor(QColor(0, 0, 0, 60));
    ui->loginFrame->setGraphicsEffect(m_shadowEffect);
    
    // 框架动画
    m_frameAnimation = new QPropertyAnimation(ui->loginFrame, "geometry");
    m_frameAnimation->setDuration(300);
    m_frameAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void LoginDialog::paintEvent(QPaintEvent *event)
{
    // 使用简单的背景色
    QDialog::paintEvent(event);
}

void LoginDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    
    // 确保输入框能正确获得焦点
    QTimer* focusTimer = new QTimer(this);
    focusTimer->setSingleShot(true);
    connect(focusTimer, &QTimer::timeout, this, [this]() {
        ui->usernameEdit->setFocus();
        ui->usernameEdit->activateWindow();
    });
    focusTimer->start(100);
    
    // 入场动画
    if (ui->loginFrame && m_frameAnimation) {
        QRect startGeometry = ui->loginFrame->geometry();
        QRect endGeometry = startGeometry;
        startGeometry.moveTop(startGeometry.top() - 50);
        
        ui->loginFrame->setGeometry(startGeometry);
        m_frameAnimation->setStartValue(startGeometry);
        m_frameAnimation->setEndValue(endGeometry);
        m_frameAnimation->start();
    }
}

void LoginDialog::onLoginClicked()
{
    if (!validateInput()) {
        return;
    }
    
    clearMessage();
    
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();
    
    // 尝试登录
    DatabaseManager* db = DatabaseManager::instance();
    if (db->loginUser(username, password, m_currentUser)) {
        // 验证角色
        QString selectedRole = ui->roleCombo->currentData().toString();
        if (m_currentUser.role != selectedRole) {
            showMessage("登录身份与账户角色不匹配！", true);
            return;
        }
        
        m_loginSuccess = true;
        showMessage("登录成功！正在跳转...", false);
        
        // 延迟关闭对话框，让用户看到成功消息
        QTimer* closeTimer = new QTimer(this);
        closeTimer->setSingleShot(true);
        connect(closeTimer, &QTimer::timeout, this, &QDialog::accept);
        closeTimer->start(1000);
    } else {
        showMessage("用户名/邮箱或密码错误！", true);
    }
}

void LoginDialog::onRegisterClicked()
{
    RegisterDialog registerDialog(this);
    registerDialog.setSelectedRole(ui->roleCombo->currentData().toString());
    
    if (registerDialog.exec() == QDialog::Accepted) {
        UserInfo newUser = registerDialog.getRegisteredUser();
        showMessage(QString("注册成功！欢迎 %1").arg(newUser.realName.isEmpty() ? newUser.username : newUser.realName), false);
        
        // 自动填充登录信息
        ui->usernameEdit->setText(newUser.username);
        ui->passwordEdit->setFocus();
    }
}

void LoginDialog::onForgotPasswordClicked()
{
    ForgotPasswordDialog forgotPasswordDialog(this);
    forgotPasswordDialog.exec();
}

void LoginDialog::onRoleChanged()
{
    clearMessage();
    // 可以根据角色预设一些默认用户名提示
    QString role = ui->roleCombo->currentData().toString();
    if (role == "管理员") {
        ui->usernameEdit->setPlaceholderText("请输入管理员账号或邮箱");
    } else if (role == "客服") {
        ui->usernameEdit->setPlaceholderText("请输入客服工号或邮箱");
    } else {
        ui->usernameEdit->setPlaceholderText("请输入用户名、邮箱或手机号");
    }
}

void LoginDialog::onUsernameChanged()
{
    clearMessage();
    // 检查是否可以启用登录按钮
    bool canLogin = !ui->usernameEdit->text().trimmed().isEmpty() && 
                    !ui->passwordEdit->text().isEmpty();
    ui->loginButton->setEnabled(canLogin);
}

void LoginDialog::onPasswordChanged()
{
    clearMessage();
    // 检查是否可以启用登录按钮
    bool canLogin = !ui->usernameEdit->text().trimmed().isEmpty() && 
                    !ui->passwordEdit->text().isEmpty();
    ui->loginButton->setEnabled(canLogin);
}

bool LoginDialog::validateInput()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();
    
    if (username.isEmpty()) {
        showMessage("请输入用户名或邮箱！", true);
        ui->usernameEdit->setFocus();
        return false;
    }
    
    if (password.isEmpty()) {
        showMessage("请输入密码！", true);
        ui->passwordEdit->setFocus();
        return false;
    }
    
    // 对于邮箱登录，不需要严格的长度检查，只对明显的用户名进行检查
    if (!username.contains("@") && username.length() < 3) {
        showMessage("用户名至少需要3个字符！", true);
        ui->usernameEdit->setFocus();
        return false;
    }
    
    // 如果是邮箱格式，进行简单的邮箱格式检查
    if (username.contains("@")) {
        QRegularExpression emailRegex("^[\\w\\.-]+@[\\w\\.-]+\\.[a-zA-Z]{2,}$");
        if (!emailRegex.match(username).hasMatch()) {
            showMessage("请输入有效的邮箱格式！", true);
            ui->usernameEdit->setFocus();
            return false;
        }
    }
    
    if (password.length() < 6) {
        showMessage("密码至少需要6个字符！", true);
        ui->passwordEdit->setFocus();
        return false;
    }
    
    return true;
}

void LoginDialog::showMessage(const QString& message, bool isError)
{
    ui->messageLabel->setText(message);
    ui->messageLabel->setProperty("error", isError);
    ui->messageLabel->style()->unpolish(ui->messageLabel);
    ui->messageLabel->style()->polish(ui->messageLabel);
    ui->messageLabel->show();
}

void LoginDialog::clearMessage()
{
    ui->messageLabel->hide();
}

// 添加键盘事件处理
void LoginDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        reject();
        return;
    }
    
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (ui->loginButton->isEnabled()) {
            onLoginClicked();
        }
        return;
    }
    
    QDialog::keyPressEvent(event);
}

// 添加关闭事件处理
void LoginDialog::closeEvent(QCloseEvent *event)
{
    // 确保能正确退出
    event->accept();
    QDialog::closeEvent(event);
} 