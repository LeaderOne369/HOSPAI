#include "LoginDialog.h"
#include "RegisterDialog.h"
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QMessageBox>
#include <QRegularExpression>
#include <QKeyEvent>
#include <QApplication>
#include <QTimer>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , m_loginSuccess(false)
{
    setWindowTitle("HospAI - 登录");
    setFixedSize(480, 720);
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_TranslucentBackground, false);
    
    setupUI();
    setupStyles();
    setupAnimations();
    
    // 设置默认焦点
    m_usernameEdit->setFocus();
    
    // 连接信号
    connect(m_usernameEdit, &QLineEdit::textChanged, this, &LoginDialog::onUsernameChanged);
    connect(m_passwordEdit, &QLineEdit::textChanged, this, &LoginDialog::onPasswordChanged);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);
}

void LoginDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // 主框架
    m_loginFrame = new QFrame;
    m_loginFrame->setObjectName("loginFrame");
    m_frameLayout = new QVBoxLayout(m_loginFrame);
    m_frameLayout->setContentsMargins(30, 30, 30, 30);
    m_frameLayout->setSpacing(20);
    
    // Logo和标题
    m_logoLabel = new QLabel("🏥");
    m_logoLabel->setAlignment(Qt::AlignCenter);
    m_logoLabel->setObjectName("logoLabel");
    
    m_titleLabel = new QLabel("HospAI");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setObjectName("titleLabel");
    
    m_subtitleLabel = new QLabel("医院智慧客服系统");
    m_subtitleLabel->setAlignment(Qt::AlignCenter);
    m_subtitleLabel->setObjectName("subtitleLabel");
    
    // 输入区域
    m_inputFrame = new QFrame;
    m_inputFrame->setObjectName("inputFrame");
    m_inputLayout = new QGridLayout(m_inputFrame);
    m_inputLayout->setSpacing(12);
    m_inputLayout->setContentsMargins(25, 25, 25, 25);
    m_inputLayout->setColumnMinimumWidth(0, 80);
    m_inputLayout->setColumnStretch(1, 1);
    
    // 角色选择
    m_roleLabel = new QLabel("登录身份:");
    m_roleLabel->setObjectName("inputLabel");
    m_roleCombo = new QComboBox;
    m_roleCombo->addItem("👨‍⚕️ 患者", "患者");
    m_roleCombo->addItem("👩‍💼 客服", "客服");
    m_roleCombo->addItem("👨‍💻 管理员", "管理员");
    m_roleCombo->setObjectName("roleCombo");
    connect(m_roleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &LoginDialog::onRoleChanged);
    
    // 用户名
    m_usernameLabel = new QLabel("用户名:");
    m_usernameLabel->setObjectName("inputLabel");
    m_usernameEdit = new QLineEdit;
    m_usernameEdit->setPlaceholderText("请输入用户名");
    m_usernameEdit->setObjectName("inputEdit");
    
    // 密码
    m_passwordLabel = new QLabel("密码:");
    m_passwordLabel->setObjectName("inputLabel");
    m_passwordEdit = new QLineEdit;
    m_passwordEdit->setPlaceholderText("请输入密码");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setObjectName("inputEdit");
    
    // 记住密码
    m_rememberCheck = new QCheckBox("记住密码");
    m_rememberCheck->setObjectName("rememberCheck");
    
    m_inputLayout->addWidget(m_roleLabel, 0, 0);
    m_inputLayout->addWidget(m_roleCombo, 0, 1);
    m_inputLayout->addWidget(m_usernameLabel, 1, 0);
    m_inputLayout->addWidget(m_usernameEdit, 1, 1);
    m_inputLayout->addWidget(m_passwordLabel, 2, 0);
    m_inputLayout->addWidget(m_passwordEdit, 2, 1);
    m_inputLayout->addWidget(m_rememberCheck, 3, 1);
    
    // 按钮区域
    m_buttonFrame = new QFrame;
    m_buttonFrame->setObjectName("buttonFrame");
    m_buttonLayout = new QVBoxLayout(m_buttonFrame);
    m_buttonLayout->setSpacing(15);
    
    m_loginButton = new QPushButton("登 录");
    m_loginButton->setObjectName("loginButton");
    m_loginButton->setEnabled(false);
    connect(m_loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    
    // 链接按钮
    m_linkLayout = new QHBoxLayout;
    m_registerButton = new QPushButton("注册账号");
    m_registerButton->setObjectName("linkButton");
    connect(m_registerButton, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);
    
    m_forgotButton = new QPushButton("忘记密码");
    m_forgotButton->setObjectName("linkButton");
    connect(m_forgotButton, &QPushButton::clicked, this, &LoginDialog::onForgotPasswordClicked);
    
    m_linkLayout->addWidget(m_registerButton);
    m_linkLayout->addStretch();
    m_linkLayout->addWidget(m_forgotButton);
    
    m_buttonLayout->addWidget(m_loginButton);
    m_buttonLayout->addLayout(m_linkLayout);
    
    // 消息标签
    m_messageLabel = new QLabel;
    m_messageLabel->setAlignment(Qt::AlignCenter);
    m_messageLabel->setObjectName("messageLabel");
    m_messageLabel->setWordWrap(true);
    m_messageLabel->hide();
    
    // 组装布局
    m_frameLayout->addWidget(m_logoLabel);
    m_frameLayout->addWidget(m_titleLabel);
    m_frameLayout->addWidget(m_subtitleLabel);
    m_frameLayout->addSpacing(10);
    m_frameLayout->addWidget(m_inputFrame);
    m_frameLayout->addWidget(m_buttonFrame);
    m_frameLayout->addWidget(m_messageLabel);
    m_frameLayout->addStretch();
    
    m_mainLayout->addWidget(m_loginFrame);
}

void LoginDialog::setupStyles()
{
    setStyleSheet(R"(
        LoginDialog {
            background-color: #f8f9fa;
        }
        
        QFrame#loginFrame {
            background-color: white;
            border: 2px solid #dee2e6;
            border-radius: 15px;
        }
        
        QLabel#logoLabel {
            font-size: 64px;
            color: #2c3e50;
            margin: 10px;
        }
        
        QLabel#titleLabel {
            font-size: 32px;
            font-weight: bold;
            color: #2c3e50;
            margin: 5px;
        }
        
        QLabel#subtitleLabel {
            font-size: 14px;
            color: #6c757d;
            margin: 5px;
        }
        
        QFrame#inputFrame {
            background-color: #f8f9fa;
            border: 1px solid #dee2e6;
            border-radius: 10px;
            padding: 10px;
        }
        
        QLabel#inputLabel {
            font-size: 14px;
            font-weight: bold;
            color: #495057;
            min-width: 80px;
            max-width: 80px;
        }
        
        QLineEdit#inputEdit {
            padding: 10px 12px;
            border: 2px solid #e9ecef;
            border-radius: 8px;
            font-size: 14px;
            background-color: white;
            min-height: 20px;
            selection-background-color: #007bff;
        }
        
        QLineEdit#inputEdit:focus {
            border-color: #007bff;
            background-color: #ffffff;
        }
        
        QComboBox#roleCombo {
            padding: 10px 12px;
            border: 2px solid #e9ecef;
            border-radius: 8px;
            font-size: 14px;
            background-color: white;
            min-height: 20px;
        }
        
        QComboBox#roleCombo:focus {
            border-color: #007bff;
        }
        
        QCheckBox#rememberCheck {
            font-size: 13px;
            color: #6c757d;
            spacing: 8px;
        }
        
        QCheckBox#rememberCheck::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid #dee2e6;
            border-radius: 4px;
            background: white;
        }
        
        QCheckBox#rememberCheck::indicator:checked {
            background: #007bff;
            border-color: #007bff;
            image: url(data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTAiIGhlaWdodD0iOCIgdmlld0JveD0iMCAwIDEwIDgiIGZpbGw9Im5vbmUiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyI+CjxwYXRoIGQ9Ik04LjU0NTk1IDAuNUwzLjU0NTk1IDUuNUwxLjQ1NDA1IDMuNDA5MDkiIHN0cm9rZT0id2hpdGUiIHN0cm9rZS13aWR0aD0iMS41IiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiLz4KPC9zdmc+);
        }
        
        QPushButton#loginButton {
            padding: 15px 30px;
            font-size: 16px;
            font-weight: bold;
            color: white;
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                      stop:0 #007bff, stop:1 #0056b3);
            border: none;
            border-radius: 12px;
            min-height: 25px;
        }
        
        QPushButton#loginButton:hover:enabled {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                      stop:0 #0056b3, stop:1 #004085);
        }
        
        QPushButton#loginButton:pressed:enabled {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                      stop:0 #004085, stop:1 #002752);
        }
        
        QPushButton#loginButton:disabled {
            background: #e9ecef;
            color: #6c757d;
        }
        
        QPushButton#linkButton {
            background: transparent;
            border: none;
            color: #007bff;
            font-size: 13px;
            text-decoration: underline;
            padding: 8px 16px;
        }
        
        QPushButton#linkButton:hover {
            color: #0056b3;
            background: rgba(0, 123, 255, 0.1);
            border-radius: 6px;
        }
        
        QLabel#messageLabel {
            padding: 10px 15px;
            border-radius: 8px;
            font-size: 13px;
            font-weight: bold;
        }
        
        QLabel#messageLabel[error="true"] {
            background: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
        
        QLabel#messageLabel[error="false"] {
            background: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }
    )");
}

void LoginDialog::setupAnimations()
{
    // 阴影效果
    m_shadowEffect = new QGraphicsDropShadowEffect;
    m_shadowEffect->setBlurRadius(20);
    m_shadowEffect->setXOffset(0);
    m_shadowEffect->setYOffset(10);
    m_shadowEffect->setColor(QColor(0, 0, 0, 60));
    m_loginFrame->setGraphicsEffect(m_shadowEffect);
    
    // 框架动画
    m_frameAnimation = new QPropertyAnimation(m_loginFrame, "geometry");
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
    QTimer::singleShot(100, this, [this]() {
        m_usernameEdit->setFocus();
        m_usernameEdit->activateWindow();
    });
    
    // 入场动画
    if (m_loginFrame && m_frameAnimation) {
        QRect startGeometry = m_loginFrame->geometry();
        QRect endGeometry = startGeometry;
        startGeometry.moveTop(startGeometry.top() - 50);
        
        m_loginFrame->setGeometry(startGeometry);
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
    
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();
    
    // 尝试登录
    DatabaseManager* db = DatabaseManager::instance();
    if (db->loginUser(username, password, m_currentUser)) {
        // 验证角色
        QString selectedRole = m_roleCombo->currentData().toString();
        if (m_currentUser.role != selectedRole) {
            showMessage("登录身份与账户角色不匹配！", true);
            return;
        }
        
        m_loginSuccess = true;
        showMessage("登录成功！正在跳转...", false);
        
        // 延迟关闭对话框，让用户看到成功消息
        QTimer::singleShot(1000, this, &QDialog::accept);
    } else {
        showMessage("用户名或密码错误！", true);
    }
}

void LoginDialog::onRegisterClicked()
{
    RegisterDialog registerDialog(this);
    registerDialog.setSelectedRole(m_roleCombo->currentData().toString());
    
    if (registerDialog.exec() == QDialog::Accepted) {
        UserInfo newUser = registerDialog.getRegisteredUser();
        showMessage(QString("注册成功！欢迎 %1").arg(newUser.realName.isEmpty() ? newUser.username : newUser.realName), false);
        
        // 自动填充登录信息
        m_usernameEdit->setText(newUser.username);
        m_passwordEdit->setFocus();
    }
}

void LoginDialog::onForgotPasswordClicked()
{
    QMessageBox::information(this, "忘记密码", 
                           "密码重置功能正在开发中...\n\n"
                           "如需重置密码，请联系系统管理员。");
}

void LoginDialog::onRoleChanged()
{
    clearMessage();
    // 可以根据角色预设一些默认用户名提示
    QString role = m_roleCombo->currentData().toString();
    if (role == "管理员") {
        m_usernameEdit->setPlaceholderText("请输入管理员账号");
    } else if (role == "客服") {
        m_usernameEdit->setPlaceholderText("请输入客服工号");
    } else {
        m_usernameEdit->setPlaceholderText("请输入用户名或手机号");
    }
}

void LoginDialog::onUsernameChanged()
{
    clearMessage();
    // 检查是否可以启用登录按钮
    bool canLogin = !m_usernameEdit->text().trimmed().isEmpty() && 
                    !m_passwordEdit->text().isEmpty();
    m_loginButton->setEnabled(canLogin);
}

void LoginDialog::onPasswordChanged()
{
    clearMessage();
    // 检查是否可以启用登录按钮
    bool canLogin = !m_usernameEdit->text().trimmed().isEmpty() && 
                    !m_passwordEdit->text().isEmpty();
    m_loginButton->setEnabled(canLogin);
}

bool LoginDialog::validateInput()
{
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();
    
    if (username.isEmpty()) {
        showMessage("请输入用户名！", true);
        m_usernameEdit->setFocus();
        return false;
    }
    
    if (password.isEmpty()) {
        showMessage("请输入密码！", true);
        m_passwordEdit->setFocus();
        return false;
    }
    
    if (username.length() < 3) {
        showMessage("用户名至少需要3个字符！", true);
        m_usernameEdit->setFocus();
        return false;
    }
    
    if (password.length() < 6) {
        showMessage("密码至少需要6个字符！", true);
        m_passwordEdit->setFocus();
        return false;
    }
    
    return true;
}

void LoginDialog::showMessage(const QString& message, bool isError)
{
    m_messageLabel->setText(message);
    m_messageLabel->setProperty("error", isError);
    m_messageLabel->style()->unpolish(m_messageLabel);
    m_messageLabel->style()->polish(m_messageLabel);
    m_messageLabel->show();
}

void LoginDialog::clearMessage()
{
    m_messageLabel->hide();
}

// 添加键盘事件处理
void LoginDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        reject();
        return;
    }
    
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (m_loginButton->isEnabled()) {
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