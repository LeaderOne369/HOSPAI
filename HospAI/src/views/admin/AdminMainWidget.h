#ifndef ADMINMAINWIDGET_H
#define ADMINMAINWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include "UserManageWidget.h"
#include "SystemStatsWidget.h"
#include "SystemConfigWidget.h"
#include "AuditLogWidget.h"

class AdminMainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AdminMainWidget(QWidget *parent = nullptr);

private:
    void setupUI();
    
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    UserManageWidget* m_userManageWidget;
    SystemStatsWidget* m_systemStatsWidget;
    SystemConfigWidget* m_systemConfigWidget;
    AuditLogWidget* m_auditLogWidget;
};

#endif // ADMINMAINWIDGET_H 