#ifndef FORGOTPASSWORDDIALOG_H
#define FORGOTPASSWORDDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include "../../core/DatabaseManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ForgotPasswordDialog; }
QT_END_NAMESPACE

class ForgotPasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ForgotPasswordDialog(QWidget *parent = nullptr);
    ~ForgotPasswordDialog();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onSendResetLinkClicked();
    void onBackToLoginClicked();
    void onInputChanged();

private:
    void setupConnections();
    void setupAnimations();
    bool validateInput();
    void showMessage(const QString& message, const QString& type = "error");
    void clearMessage();
    
    Ui::ForgotPasswordDialog *ui;
    
    // 动画效果
    QPropertyAnimation* m_frameAnimation;
    QGraphicsDropShadowEffect* m_shadowEffect;
};

#endif // FORGOTPASSWORDDIALOG_H 