#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QTimer>
#include "../../core/DatabaseManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LoginDialog; }
QT_END_NAMESPACE

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();
    
    UserInfo getLoggedInUser() const { return m_currentUser; }
    bool isLoginSuccessful() const { return m_loginSuccess; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onForgotPasswordClicked();
    void onRoleChanged();
    void onUsernameChanged();
    void onPasswordChanged();

private:
    void setupConnections();
    void setupAnimations();
    bool validateInput();
    void showMessage(const QString& message, bool isError = false);
    void clearMessage();
    
    Ui::LoginDialog *ui;
    
    // 动画效果
    QPropertyAnimation* m_frameAnimation;
    QGraphicsDropShadowEffect* m_shadowEffect;
    
    // 数据
    UserInfo m_currentUser;
    bool m_loginSuccess;
};

#endif // LOGINDIALOG_H 