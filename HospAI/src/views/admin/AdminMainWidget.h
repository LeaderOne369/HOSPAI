#ifndef ADMINMAINWIDGET_H
#define ADMINMAINWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include "UserManageWidget.h"
#include "PatientManageWidget.h"
#include "QuickReplyManageWidget.h"
#include "StaffRatingWidget.h"
#include "SystemStatsWidget.h"
#include "SystemConfigWidget.h"
#include "AuditLogWidget.h"
#include "../../core/DatabaseManager.h"

class AdminMainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AdminMainWidget(QWidget *parent = nullptr);
    
    void setCurrentUser(const UserInfo& user);
    void setDatabaseManager(DatabaseManager* dbManager);

private:
    void setupUI();
    
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    UserManageWidget* m_userManageWidget;
    SystemStatsWidget* m_systemStatsWidget;
    SystemConfigWidget* m_systemConfigWidget;
    AuditLogWidget* m_auditLogWidget;
    
    PatientManageWidget* m_patientManageWidget;
    QuickReplyManageWidget* m_quickReplyManageWidget;
    StaffRatingWidget* m_staffRatingWidget;
    
    UserInfo m_currentUser;
    DatabaseManager* m_dbManager;
};

#endif // ADMINMAINWIDGET_H 