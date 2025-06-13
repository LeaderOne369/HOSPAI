#include "PatientManageWidget.h"
#include "../common/UIStyleManager.h"
#include <QSplitter>
#include <QDateTime>

PatientManageWidget::PatientManageWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_dbManager(nullptr)
{
    setupUI();
}

void PatientManageWidget::setDatabaseManager(DatabaseManager* dbManager)
{
    m_dbManager = dbManager;
    loadPatients();
}

void PatientManageWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    m_mainLayout->setSpacing(15);
    
    // 标题
    QLabel* titleLabel = new QLabel("患者管理", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2C3E50; margin-bottom: 10px;");
    m_mainLayout->addWidget(titleLabel);
    
    // 顶部工具栏
    m_toolLayout = new QHBoxLayout;
    m_toolLayout->setSpacing(10);
    
    m_btnAdd = new QPushButton("➕ 新增患者", this);
    m_btnEdit = new QPushButton("✏️ 编辑患者", this);
    m_btnDelete = new QPushButton("🗑️ 删除患者", this);
    m_btnRefresh = new QPushButton("🔄 刷新", this);
    m_btnExport = new QPushButton("📤 导出", this);
    m_btnViewHistory = new QPushButton("📋 查看历史", this);
    
    UIStyleManager::applyButtonStyle(m_btnAdd, "primary");
    UIStyleManager::applyButtonStyle(m_btnEdit, "secondary");
    UIStyleManager::applyButtonStyle(m_btnDelete, "error");
    UIStyleManager::applyButtonStyle(m_btnRefresh, "secondary");
    UIStyleManager::applyButtonStyle(m_btnExport, "secondary");
    UIStyleManager::applyButtonStyle(m_btnViewHistory, "secondary");
    
    m_toolLayout->addWidget(m_btnAdd);
    m_toolLayout->addWidget(m_btnEdit);
    m_toolLayout->addWidget(m_btnDelete);
    m_toolLayout->addWidget(m_btnRefresh);
    m_toolLayout->addWidget(m_btnExport);
    m_toolLayout->addWidget(m_btnViewHistory);
    m_toolLayout->addStretch();
    
    m_mainLayout->addLayout(m_toolLayout);
    
    // 搜索区域
    m_searchGroup = new QGroupBox("搜索条件", this);
    UIStyleManager::applyGroupBoxStyle(m_searchGroup);
    
    QHBoxLayout* searchLayout = new QHBoxLayout(m_searchGroup);
    searchLayout->setSpacing(10);
    
    searchLayout->addWidget(new QLabel("关键字:", this));
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("搜索用户名、真实姓名、电话或邮箱...");
    UIStyleManager::applyLineEditStyle(m_searchEdit);
    searchLayout->addWidget(m_searchEdit);
    
    m_btnSearch = new QPushButton("🔍 搜索", this);
    UIStyleManager::applyButtonStyle(m_btnSearch, "primary");
    searchLayout->addWidget(m_btnSearch);
    
    m_mainLayout->addWidget(m_searchGroup);
    
    // 患者列表表格
    m_patientTable = new QTableWidget(this);
    m_patientTable->setColumnCount(6);
    QStringList headers = {"用户名", "真实姓名", "邮箱", "电话", "注册时间", "状态"};
    m_patientTable->setHorizontalHeaderLabels(headers);
    
    // 设置表格属性
    m_patientTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_patientTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_patientTable->setAlternatingRowColors(true);
    m_patientTable->setSortingEnabled(true);
    
    // UIStyleManager::applyTableStyle(m_patientTable); // 暂时注释掉
    m_mainLayout->addWidget(m_patientTable);
    
    // 统计信息
    m_statsLabel = new QLabel("共 0 位患者", this);
    m_statsLabel->setStyleSheet("color: #7F8C8D; font-size: 12px;");
    m_mainLayout->addWidget(m_statsLabel);
    
    // 连接信号槽
    connect(m_btnRefresh, &QPushButton::clicked, this, &PatientManageWidget::onRefreshPatients);
    connect(m_btnAdd, &QPushButton::clicked, [this]() { QMessageBox::information(this, "提示", "添加患者功能待实现"); });
    connect(m_btnEdit, &QPushButton::clicked, [this]() { QMessageBox::information(this, "提示", "编辑患者功能待实现"); });
    connect(m_btnDelete, &QPushButton::clicked, [this]() { QMessageBox::information(this, "提示", "删除患者功能待实现"); });
    connect(m_btnExport, &QPushButton::clicked, [this]() { QMessageBox::information(this, "提示", "导出患者功能待实现"); });
    connect(m_btnViewHistory, &QPushButton::clicked, [this]() { QMessageBox::information(this, "提示", "查看患者历史功能待实现"); });
    connect(m_btnSearch, &QPushButton::clicked, this, &PatientManageWidget::onSearchPatients);
}

void PatientManageWidget::loadPatients()
{
    if (!m_dbManager) return;
    
    m_patients = m_dbManager->getUsersByRole("患者");
    
    m_patientTable->setRowCount(m_patients.size());
    
    for (int i = 0; i < m_patients.size(); ++i) {
        addPatientToTable(i, m_patients[i]);
    }
    
    // 更新统计信息
    m_statsLabel->setText(QString("共 %1 位患者").arg(m_patients.size()));
}

void PatientManageWidget::addPatientToTable(int row, const UserInfo& patient)
{
    m_patientTable->setItem(row, 0, new QTableWidgetItem(patient.username));
    m_patientTable->setItem(row, 1, new QTableWidgetItem(patient.realName));
    m_patientTable->setItem(row, 2, new QTableWidgetItem(patient.email));
    m_patientTable->setItem(row, 3, new QTableWidgetItem(patient.phone));
    m_patientTable->setItem(row, 4, new QTableWidgetItem(patient.createdAt.toString("yyyy-MM-dd hh:mm")));
    
    QString statusText = patient.status == 1 ? "启用" : "禁用";
    QTableWidgetItem* statusItem = new QTableWidgetItem(statusText);
    statusItem->setForeground(patient.status == 1 ? QColor("#27AE60") : QColor("#E74C3C"));
    m_patientTable->setItem(row, 5, statusItem);
}

void PatientManageWidget::onRefreshPatients()
{
    loadPatients();
}

void PatientManageWidget::onSearchPatients()
{
    QString keyword = m_searchEdit->text().trimmed().toLower();
    
    for (int i = 0; i < m_patientTable->rowCount(); ++i) {
        bool show = true;
        
        if (!keyword.isEmpty()) {
            bool found = false;
            for (int j = 0; j < 4; ++j) {
                QTableWidgetItem* item = m_patientTable->item(i, j);
                if (item && item->text().toLower().contains(keyword)) {
                    found = true;
                    break;
                }
            }
            if (!found) show = false;
        }
        
        m_patientTable->setRowHidden(i, !show);
    }
}

// 其他槽函数的简化实现
void PatientManageWidget::onAddPatient() { QMessageBox::information(this, "提示", "功能开发中..."); }
void PatientManageWidget::onEditPatient() { QMessageBox::information(this, "提示", "功能开发中..."); }
void PatientManageWidget::onDeletePatient() { QMessageBox::information(this, "提示", "功能开发中..."); }
void PatientManageWidget::onPatientSelectionChanged() { /* 暂时留空 */ }
void PatientManageWidget::onExportPatients() { QMessageBox::information(this, "提示", "功能开发中..."); }
void PatientManageWidget::onViewPatientHistory() { QMessageBox::information(this, "提示", "功能开发中..."); }
void PatientManageWidget::showPatientDialog(const UserInfo& patient) { Q_UNUSED(patient); QMessageBox::information(this, "提示", "功能开发中..."); }

// ========== PatientEditDialog 简化实现 ==========

PatientEditDialog::PatientEditDialog(const UserInfo& patient, QWidget *parent)
    : QDialog(parent)
    , m_originalPatient(patient)
    , m_isEditing(patient.id > 0)
{
    setWindowTitle("患者编辑对话框（开发中）");
    resize(300, 200);
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("患者编辑功能正在开发中...", this));
    
    QPushButton* closeButton = new QPushButton("关闭", this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);
    layout->addWidget(closeButton);
}

void PatientEditDialog::setupUI() { /* 简化实现 */ }
void PatientEditDialog::onAccept() { accept(); }

UserInfo PatientEditDialog::getPatientInfo() const
{
    return m_originalPatient;
}

// MOC generated automatically 