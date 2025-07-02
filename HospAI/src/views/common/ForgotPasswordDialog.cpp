#include "ForgotPasswordDialog.h"
#include "ui_ForgotPasswordDialog.h"
#include <QRegularExpression>
#include <QShowEvent>

ForgotPasswordDialog::ForgotPasswordDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ForgotPasswordDialog)
    , m_frameAnimation(nullptr)
    , m_shadowEffect(nullptr)
{
    ui->setupUi(this);
    
    // 设置窗口属性
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_TranslucentBackground, false);
    setFixedSize(600, 700);
    
    setupConnections();
    setupAnimations();
    
    // 初始隐藏消息标签
    ui->messageLabel->hide();
    
    // 设置焦点
    ui->usernameEdit->setFocus();
}

ForgotPasswordDialog::~ForgotPasswordDialog()
{
    delete ui;
}

void ForgotPasswordDialog::setupConnections()
{
    connect(ui->usernameEdit, &QLineEdit::textChanged, this, &ForgotPasswordDialog::onInputChanged);
    connect(ui->emailEdit, &QLineEdit::textChanged, this, &ForgotPasswordDialog::onInputChanged);
    
    connect(ui->emailEdit, &QLineEdit::returnPressed, this, &ForgotPasswordDialog::onSendResetLinkClicked);
    connect(ui->resetBtn, &QPushButton::clicked, this, &ForgotPasswordDialog::onSendResetLinkClicked);
    connect(ui->backLink, &QPushButton::clicked, this, &ForgotPasswordDialog::onBackToLoginClicked);
}

void ForgotPasswordDialog::setupAnimations()
{
    // 阴影效果
    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setBlurRadius(20);
    m_shadowEffect->setXOffset(0);
    m_shadowEffect->setYOffset(4);
    m_shadowEffect->setColor(QColor(0, 0, 0, 100));
    ui->resetContainer->setGraphicsEffect(m_shadowEffect);
    
    // 框架动画
    m_frameAnimation = new QPropertyAnimation(ui->resetContainer, "geometry");
    m_frameAnimation->setDuration(300);
    m_frameAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void ForgotPasswordDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    
    // 设置焦点
    QTimer::singleShot(100, this, [this]() {
        ui->usernameEdit->setFocus();
        ui->usernameEdit->activateWindow();
    });
    
    // 入场动画
    if (m_frameAnimation) {
        QRect startGeometry = ui->resetContainer->geometry();
        QRect endGeometry = startGeometry;
        startGeometry.moveTop(startGeometry.top() - 40);
        
        ui->resetContainer->setGeometry(startGeometry);
        m_frameAnimation->setStartValue(startGeometry);
        m_frameAnimation->setEndValue(endGeometry);
        m_frameAnimation->start();
    }
}

void ForgotPasswordDialog::onSendResetLinkClicked()
{
    if (!validateInput()) {
        return;
    }
    
    clearMessage();
    
    QString username = ui->usernameEdit->text().trimmed();
    QString email = ui->emailEdit->text().trimmed();
    
    DatabaseManager* db = DatabaseManager::instance();
    
    // 检查用户名是否存在
    UserInfo userByUsername = db->getUserByUsername(username);
    if (userByUsername.id <= 0) {
        showMessage("用户名不存在，请检查后重试。", "error");
        ui->usernameEdit->setFocus();
        return;
    }
    
    // 检查邮箱是否与用户名匹配
    if (userByUsername.email.isEmpty()) {
        showMessage("该用户未绑定邮箱，无法发送重置链接。", "error");
        return;
    }
    
    if (userByUsername.email.toLower() != email.toLower()) {
        showMessage("邮箱与用户名不匹配，请检查后重试。", "error");
        ui->emailEdit->setFocus();
        return;
    }
    
    // 模拟发送重置链接（这里可以集成真实的邮件服务）
    showMessage("重置链接已发送至邮箱，请查收。", "success");
    
    // 禁用按钮一段时间，防止重复发送
    ui->resetBtn->setEnabled(false);
    ui->resetBtn->setText("已发送，请查收邮箱");
    
    QTimer::singleShot(30000, this, [this]() {  // 30秒后重新启用
        ui->resetBtn->setEnabled(true);
        ui->resetBtn->setText("发送重置链接");
        onInputChanged(); // 重新检查输入状态
    });
    
    // 3秒后自动关闭对话框
    QTimer::singleShot(3000, this, [this]() {
        accept();
    });
}

void ForgotPasswordDialog::onBackToLoginClicked()
{
    reject();
}

void ForgotPasswordDialog::onInputChanged()
{
    clearMessage();
    
    // 检查是否可以启用发送按钮
    bool canSend = !ui->usernameEdit->text().trimmed().isEmpty() &&
                   !ui->emailEdit->text().trimmed().isEmpty();
    
    // 如果按钮文本被改变了（表示刚发送过），保持禁用状态
    if (ui->resetBtn->text() != "发送重置链接") {
        canSend = false;
    }
    
    ui->resetBtn->setEnabled(canSend);
    
    // 实时邮箱格式验证
    QString email = ui->emailEdit->text().trimmed();
    if (!email.isEmpty()) {
        QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
        if (!emailRegex.match(email).hasMatch()) {
            ui->emailEdit->setStyleSheet("QLineEdit { border-color: #ff6b6b; }");
        } else {
            ui->emailEdit->setStyleSheet("");
        }
    } else {
        ui->emailEdit->setStyleSheet("");
    }
}

bool ForgotPasswordDialog::validateInput()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString email = ui->emailEdit->text().trimmed();
    
    if (username.isEmpty()) {
        showMessage("请输入用户名！", "error");
        ui->usernameEdit->setFocus();
        return false;
    }
    
    if (email.isEmpty()) {
        showMessage("请输入邮箱地址！", "error");
        ui->emailEdit->setFocus();
        return false;
    }
    
    // 验证邮箱格式
    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    if (!emailRegex.match(email).hasMatch()) {
        showMessage("邮箱格式不正确！", "error");
        ui->emailEdit->setFocus();
        return false;
    }
    
    return true;
}

void ForgotPasswordDialog::showMessage(const QString& message, const QString& type)
{
    ui->messageLabel->setText(message);
    ui->messageLabel->setProperty("type", type);
    
    if (type == "success") {
        ui->messageLabel->setStyleSheet(R"(
            QLabel#messageLabel[type="success"] {
                background-color: #e8f5e8;
                color: #2e7d32;
                border: 1px solid #c8e6c9;
                padding: 10px;
                border-radius: 6px;
                font-size: 14px;
                margin-top: 20px;
            }
        )");
    } else {
        ui->messageLabel->setStyleSheet(R"(
            QLabel#messageLabel[type="error"] {
                background-color: #ffebee;
                color: #c62828;
                border: 1px solid #ffcdd2;
                padding: 10px;
                border-radius: 6px;
                font-size: 14px;
                margin-top: 20px;
            }
        )");
    }
    
    ui->messageLabel->show();
}

void ForgotPasswordDialog::clearMessage()
{
    ui->messageLabel->hide();
} 