#ifndef PATIENTMANAGEWIDGET_H
#define PATIENTMANAGEWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QDateTimeEdit>
#include <QCheckBox>
#include <QDialog>
#include <QFormLayout>
#include <QSpinBox>
#include <QTextEdit>
#include <QHeaderView>
#include <QMessageBox>
#include "../../core/DatabaseManager.h"

class PatientManageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PatientManageWidget(QWidget *parent = nullptr);
    void setDatabaseManager(DatabaseManager* dbManager);

private slots:
    void onAddPatient();
    void onEditPatient();
    void onDeletePatient();
    void onRefreshPatients();
    void onSearchPatients();
    void onPatientSelectionChanged();
    void onExportPatients();
    void onViewPatientHistory();

private:
    void setupUI();
    void loadPatients();
    void addPatientToTable(int row, const UserInfo& patient);
    void showPatientDialog(const UserInfo& patient = UserInfo());

private:
    QVBoxLayout* m_mainLayout;
    
    // 顶部工具栏
    QHBoxLayout* m_toolLayout;
    QPushButton* m_btnAdd;
    QPushButton* m_btnEdit;
    QPushButton* m_btnDelete;
    QPushButton* m_btnRefresh;
    QPushButton* m_btnExport;
    QPushButton* m_btnViewHistory;
    
    // 搜索区域
    QGroupBox* m_searchGroup;
    QLineEdit* m_searchEdit;
    QComboBox* m_statusFilter;
    QPushButton* m_btnSearch;
    
    // 患者列表
    QTableWidget* m_patientTable;
    
    // 统计信息
    QLabel* m_statsLabel;
    
    // 数据
    DatabaseManager* m_dbManager;
    QList<UserInfo> m_patients;
};

// 患者编辑对话框
class PatientEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PatientEditDialog(const UserInfo& patient = UserInfo(), QWidget *parent = nullptr);
    UserInfo getPatientInfo() const;

private slots:
    void onAccept();

private:
    void setupUI();

private:
    QFormLayout* m_formLayout;
    QLineEdit* m_usernameEdit;
    QLineEdit* m_realNameEdit;
    QLineEdit* m_emailEdit;
    QLineEdit* m_phoneEdit;
    QLineEdit* m_passwordEdit;
    QComboBox* m_statusCombo;
    QTextEdit* m_notesEdit;
    
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    
    UserInfo m_originalPatient;
    bool m_isEditing;
};

#endif // PATIENTMANAGEWIDGET_H 