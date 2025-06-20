#ifndef SYSTEMCONFIGWIDGET_H
#define SYSTEMCONFIGWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QSlider>
#include <QDialog>
#include <QMessageBox>

class SystemConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SystemConfigWidget(QWidget *parent = nullptr);

private slots:
    void onSaveConfig();
    void onResetConfig();
    void onImportConfig();
    void onExportConfig();
    void onAddFAQ();
    void onEditFAQ();
    void onDeleteFAQ();
    void onAddDepartment();
    void onEditDepartment();
    void onDeleteDepartment();

private:
    void setupUI();
    void setupGeneralTab();
    void setupAIConfigTab();
    void setupFAQTab();
    void setupDepartmentTab();
    void loadConfig();
    void saveConfig();

private:
    QVBoxLayout* m_mainLayout;
    
    // 顶部按钮
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_btnSave;
    QPushButton* m_btnReset;
    QPushButton* m_btnImport;
    QPushButton* m_btnExport;
    
    // 选项卡
    QTabWidget* m_tabWidget;
    
    // 常规设置选项卡
    QWidget* m_generalTab;
    QLineEdit* m_systemName;
    QLineEdit* m_systemVersion;
    QSpinBox* m_maxUsers;
    QSpinBox* m_sessionTimeout;
    QCheckBox* m_enableLogging;
    QCheckBox* m_enableBackup;
    QComboBox* m_logLevel;
    
    // AI配置选项卡
    QWidget* m_aiConfigTab;
    QSlider* m_aiConfidence;
    QSpinBox* m_maxResponse;
    QCheckBox* m_enableAutoReply;
    QCheckBox* m_enableLearning;
    QTextEdit* m_aiPrompt;
    QLineEdit* m_apiEndpoint;
    QLineEdit* m_apiKey;
    
    // FAQ管理选项卡
    QWidget* m_faqTab;
    QTableWidget* m_faqTable;
    QPushButton* m_btnAddFAQ;
    QPushButton* m_btnEditFAQ;
    QPushButton* m_btnDeleteFAQ;
    
    // 科室管理选项卡
    QWidget* m_departmentTab;
    QTableWidget* m_departmentTable;
    QPushButton* m_btnAddDept;
    QPushButton* m_btnEditDept;
    QPushButton* m_btnDeleteDept;
};

// FAQ数据结构
struct FAQItem {
    int id;
    QString question;
    QString answer;
    QString category;
    int priority;
    bool enabled;
    QString keywords;
    QString createTime;
    QString updateTime;
};

// FAQ编辑对话框
class FAQEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FAQEditDialog(QWidget *parent = nullptr);
    explicit FAQEditDialog(const FAQItem& item, QWidget *parent = nullptr);

    FAQItem getFAQItem() const;
    void setFAQItem(const FAQItem& item);

signals:
    void faqSaved(const FAQItem& item);

private slots:
    void onOkClicked();
    void onCancelClicked();
    void onPreviewClicked();

private:
    void setupUI();
    void connectSignals();
    bool validateInput();

    QLineEdit* m_editQuestion;
    QTextEdit* m_editAnswer;
    QComboBox* m_comboCategory;
    QLineEdit* m_editKeywords;
    QSpinBox* m_spinPriority;
    QCheckBox* m_checkEnabled;
    
    QPushButton* m_btnOk;
    QPushButton* m_btnCancel;
    QPushButton* m_btnPreview;
    
    FAQItem m_currentItem;
    bool m_isEditMode;
};

// 科室编辑对话框
class DepartmentEditDialog : public QDialog
{
    Q_OBJECT

public:
    struct DepartmentInfo {
        int id;
        QString name;
        QString location;
        QString phone;
        QString description;
        QString workTime;
        QString director;
        bool isActive;
    };

    explicit DepartmentEditDialog(QWidget *parent = nullptr);
    explicit DepartmentEditDialog(const DepartmentInfo& dept, QWidget *parent = nullptr);

    DepartmentInfo getDepartmentInfo() const;
    void setDepartmentInfo(const DepartmentInfo& dept);

signals:
    void departmentSaved(const DepartmentInfo& dept);

private slots:
    void onOkClicked();
    void onCancelClicked();

private:
    void setupUI();
    void connectSignals();
    bool validateInput();

    QLineEdit* m_editName;
    QLineEdit* m_editLocation;
    QLineEdit* m_editPhone;
    QTextEdit* m_editDescription;
    QLineEdit* m_editWorkTime;
    QLineEdit* m_editDirector;
    QCheckBox* m_checkActive;
    
    QPushButton* m_btnOk;
    QPushButton* m_btnCancel;
    
    DepartmentInfo m_currentDept;
    bool m_isEditMode;
};

#endif // SYSTEMCONFIGWIDGET_H 