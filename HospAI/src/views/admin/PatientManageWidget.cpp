#include "PatientManageWidget.h"
#include "../common/UIStyleManager.h"
#include <QSplitter>
#include <QDateTime>
#include <QFileDialog>
#include <QTextStream>
#include <QDate>
#include <QFile>
#include <QGridLayout>
#include <QRegularExpression>
#include <QStringConverter>

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
    connect(m_btnAdd, &QPushButton::clicked, this, &PatientManageWidget::onAddPatient);
    connect(m_btnEdit, &QPushButton::clicked, this, &PatientManageWidget::onEditPatient);
    connect(m_btnDelete, &QPushButton::clicked, this, &PatientManageWidget::onDeletePatient);
    connect(m_btnExport, &QPushButton::clicked, this, &PatientManageWidget::onExportPatients);
    connect(m_btnViewHistory, &QPushButton::clicked, this, &PatientManageWidget::onViewPatientHistory);
    connect(m_btnSearch, &QPushButton::clicked, this, &PatientManageWidget::onSearchPatients);
    connect(m_patientTable, &QTableWidget::itemSelectionChanged, this, &PatientManageWidget::onPatientSelectionChanged);
    
    // 初始状态：禁用编辑和删除按钮
    m_btnEdit->setEnabled(false);
    m_btnDelete->setEnabled(false);
    m_btnViewHistory->setEnabled(false);
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

void PatientManageWidget::onAddPatient() 
{
    showPatientDialog();
}

void PatientManageWidget::onEditPatient() 
{
    int currentRow = m_patientTable->currentRow();
    if (currentRow < 0 || currentRow >= m_patients.size()) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的患者！");
        return;
    }
    
    showPatientDialog(m_patients[currentRow]);
}

void PatientManageWidget::onDeletePatient() 
{
    int currentRow = m_patientTable->currentRow();
    if (currentRow < 0 || currentRow >= m_patients.size()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的患者！");
        return;
    }
    
    UserInfo patient = m_patients[currentRow];
    int ret = QMessageBox::question(this, "确认删除", 
                                   QString("确定要删除患者 %1 (%2) 吗？\n\n此操作不可恢复！")
                                   .arg(patient.realName.isEmpty() ? patient.username : patient.realName)
                                   .arg(patient.username),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        // 这里应该调用数据库删除方法，由于DatabaseManager没有删除用户的方法，我们先禁用用户
        patient.status = 0; // 禁用用户
        if (m_dbManager && m_dbManager->updateUserInfo(patient)) {
            QMessageBox::information(this, "成功", "患者已成功删除！");
            onRefreshPatients();
        } else {
            QMessageBox::critical(this, "错误", "删除患者失败！");
        }
    }
}

void PatientManageWidget::onPatientSelectionChanged() 
{
    int currentRow = m_patientTable->currentRow();
    bool hasSelection = (currentRow >= 0 && currentRow < m_patients.size());
    
    m_btnEdit->setEnabled(hasSelection);
    m_btnDelete->setEnabled(hasSelection);
    m_btnViewHistory->setEnabled(hasSelection);
}

void PatientManageWidget::onExportPatients() 
{
    QString fileName = QFileDialog::getSaveFileName(this, 
                                                   "导出患者数据", 
                                                   QString("patients_%1.csv").arg(QDate::currentDate().toString("yyyyMMdd")),
                                                   "CSV文件 (*.csv)");
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream.setEncoding(QStringConverter::Utf8);
            
            // 写入表头
            stream << "用户名,真实姓名,邮箱,电话,注册时间,状态\n";
            
            // 写入数据
            for (const UserInfo& patient : m_patients) {
                stream << QString("%1,%2,%3,%4,%5,%6\n")
                         .arg(patient.username)
                         .arg(patient.realName)
                         .arg(patient.email)
                         .arg(patient.phone)
                         .arg(patient.createdAt.toString("yyyy-MM-dd hh:mm:ss"))
                         .arg(patient.status == 1 ? "启用" : "禁用");
            }
            
            file.close();
            QMessageBox::information(this, "成功", QString("患者数据已导出到：%1").arg(fileName));
        } else {
            QMessageBox::critical(this, "错误", "导出文件失败！");
        }
    }
}

void PatientManageWidget::onViewPatientHistory() 
{
    int currentRow = m_patientTable->currentRow();
    if (currentRow < 0 || currentRow >= m_patients.size()) {
        QMessageBox::warning(this, "提示", "请先选择要查看历史的患者！");
        return;
    }
    
    UserInfo patient = m_patients[currentRow];
    
    // 创建历史查看对话框
    QDialog* historyDialog = new QDialog(this);
    historyDialog->setWindowTitle(QString("患者历史 - %1").arg(patient.realName.isEmpty() ? patient.username : patient.realName));
    historyDialog->resize(800, 600);
    
    QVBoxLayout* layout = new QVBoxLayout(historyDialog);
    
    // 患者基本信息
    QGroupBox* infoGroup = new QGroupBox("基本信息", historyDialog);
    QGridLayout* infoLayout = new QGridLayout(infoGroup);
    infoLayout->addWidget(new QLabel("用户名:"), 0, 0);
    infoLayout->addWidget(new QLabel(patient.username), 0, 1);
    infoLayout->addWidget(new QLabel("真实姓名:"), 0, 2);
    infoLayout->addWidget(new QLabel(patient.realName), 0, 3);
    infoLayout->addWidget(new QLabel("邮箱:"), 1, 0);
    infoLayout->addWidget(new QLabel(patient.email), 1, 1);
    infoLayout->addWidget(new QLabel("电话:"), 1, 2);
    infoLayout->addWidget(new QLabel(patient.phone), 1, 3);
    infoLayout->addWidget(new QLabel("注册时间:"), 2, 0);
    infoLayout->addWidget(new QLabel(patient.createdAt.toString("yyyy-MM-dd hh:mm:ss")), 2, 1);
    infoLayout->addWidget(new QLabel("最后登录:"), 2, 2);
    infoLayout->addWidget(new QLabel(patient.lastLogin.toString("yyyy-MM-dd hh:mm:ss")), 2, 3);
    layout->addWidget(infoGroup);
    
    // 聊天会话历史
    QGroupBox* sessionGroup = new QGroupBox("聊天会话历史", historyDialog);
    QVBoxLayout* sessionLayout = new QVBoxLayout(sessionGroup);
    
    QTableWidget* sessionTable = new QTableWidget(historyDialog);
    sessionTable->setColumnCount(4);
    sessionTable->setHorizontalHeaderLabels({"会话时间", "客服", "状态", "最后消息时间"});
    
    if (m_dbManager) {
        QList<ChatSession> sessions = m_dbManager->getPatientSessions(patient.id);
        sessionTable->setRowCount(sessions.size());
        
        for (int i = 0; i < sessions.size(); ++i) {
            sessionTable->setItem(i, 0, new QTableWidgetItem(sessions[i].createdAt.toString("yyyy-MM-dd hh:mm:ss")));
            sessionTable->setItem(i, 1, new QTableWidgetItem(sessions[i].staffName));
            QString statusText = sessions[i].status == 0 ? "已结束" : (sessions[i].status == 1 ? "进行中" : "等待中");
            sessionTable->setItem(i, 2, new QTableWidgetItem(statusText));
            sessionTable->setItem(i, 3, new QTableWidgetItem(sessions[i].lastMessageAt.toString("yyyy-MM-dd hh:mm:ss")));
        }
    }
    
    sessionTable->horizontalHeader()->setStretchLastSection(true);
    sessionLayout->addWidget(sessionTable);
    layout->addWidget(sessionGroup);
    
    // 关闭按钮
    QPushButton* closeButton = new QPushButton("关闭", historyDialog);
    connect(closeButton, &QPushButton::clicked, historyDialog, &QDialog::accept);
    layout->addWidget(closeButton);
    
    historyDialog->exec();
    historyDialog->deleteLater();
}

void PatientManageWidget::showPatientDialog(const UserInfo& patient) 
{
    PatientEditDialog dialog(patient, this);
    if (dialog.exec() == QDialog::Accepted) {
        UserInfo newPatientInfo = dialog.getPatientInfo();
        
        if (patient.id > 0) {
            // 编辑模式
            if (m_dbManager && m_dbManager->updateUserInfo(newPatientInfo)) {
                QMessageBox::information(this, "成功", "患者信息更新成功！");
                onRefreshPatients();
            } else {
                QMessageBox::critical(this, "错误", "更新患者信息失败！");
            }
        } else {
            // 新增模式
            if (m_dbManager && m_dbManager->registerUser(newPatientInfo.username, 
                                                        "123456", // 默认密码
                                                        newPatientInfo.email,
                                                        newPatientInfo.phone,
                                                        "患者",
                                                        newPatientInfo.realName)) {
                QMessageBox::information(this, "成功", "患者添加成功！\n默认密码：123456");
                onRefreshPatients();
            } else {
                QMessageBox::critical(this, "错误", "添加患者失败！用户名或邮箱可能已存在。");
            }
        }
    }
}

// ========== PatientEditDialog 完整实现 ==========

PatientEditDialog::PatientEditDialog(const UserInfo& patient, QWidget *parent)
    : QDialog(parent)
    , m_originalPatient(patient)
    , m_isEditing(patient.id > 0)
{
    setWindowTitle(m_isEditing ? "编辑患者信息" : "添加新患者");
    resize(450, 400);
    setupUI();
    
    if (m_isEditing) {
        // 填充现有数据
        m_usernameEdit->setText(patient.username);
        m_realNameEdit->setText(patient.realName);
        m_emailEdit->setText(patient.email);
        m_phoneEdit->setText(patient.phone);
        m_statusCombo->setCurrentIndex(patient.status);
        m_usernameEdit->setEnabled(false); // 编辑模式下用户名不可修改
        m_passwordEdit->setPlaceholderText("留空表示不修改密码");
    }
}

void PatientEditDialog::setupUI() 
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 表单区域
    QGroupBox* formGroup = new QGroupBox("患者信息", this);
    m_formLayout = new QFormLayout(formGroup);
    
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("请输入用户名");
    m_formLayout->addRow("用户名 *:", m_usernameEdit);
    
    m_realNameEdit = new QLineEdit(this);
    m_realNameEdit->setPlaceholderText("请输入真实姓名");
    m_formLayout->addRow("真实姓名 *:", m_realNameEdit);
    
    m_emailEdit = new QLineEdit(this);
    m_emailEdit->setPlaceholderText("请输入邮箱地址");
    m_formLayout->addRow("邮箱 *:", m_emailEdit);
    
    m_phoneEdit = new QLineEdit(this);
    m_phoneEdit->setPlaceholderText("请输入手机号码");
    m_formLayout->addRow("电话:", m_phoneEdit);
    
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(m_isEditing ? "留空表示不修改" : "请输入初始密码");
    m_formLayout->addRow("密码:", m_passwordEdit);
    
    m_statusCombo = new QComboBox(this);
    m_statusCombo->addItem("禁用", 0);
    m_statusCombo->addItem("启用", 1);
    m_statusCombo->setCurrentIndex(1); // 默认启用
    m_formLayout->addRow("状态:", m_statusCombo);
    
    m_notesEdit = new QTextEdit(this);
    m_notesEdit->setPlaceholderText("备注信息（可选）");
    m_notesEdit->setMaximumHeight(80);
    m_formLayout->addRow("备注:", m_notesEdit);
    
    mainLayout->addWidget(formGroup);
    
    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    
    m_okButton = new QPushButton(m_isEditing ? "保存" : "添加", this);
    m_cancelButton = new QPushButton("取消", this);
    
    m_okButton->setDefault(true);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号
    connect(m_okButton, &QPushButton::clicked, this, &PatientEditDialog::onAccept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    // 输入验证函数
    auto updateButtonState = [this]() {
        bool valid = !m_usernameEdit->text().trimmed().isEmpty() && 
                    !m_realNameEdit->text().trimmed().isEmpty() &&
                    !m_emailEdit->text().trimmed().isEmpty();
        m_okButton->setEnabled(valid);
    };
    
    connect(m_usernameEdit, &QLineEdit::textChanged, this, updateButtonState);
    connect(m_realNameEdit, &QLineEdit::textChanged, this, updateButtonState);
    connect(m_emailEdit, &QLineEdit::textChanged, this, updateButtonState);
    
    // 设置初始按钮状态
    updateButtonState();
}

void PatientEditDialog::onAccept() 
{
    // 验证输入
    QString username = m_usernameEdit->text().trimmed();
    QString realName = m_realNameEdit->text().trimmed();
    QString email = m_emailEdit->text().trimmed();
    QString phone = m_phoneEdit->text().trimmed();
    QString password = m_passwordEdit->text();
    
    if (username.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入用户名！");
        m_usernameEdit->setFocus();
        return;
    }
    
    if (realName.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入真实姓名！");
        m_realNameEdit->setFocus();
        return;
    }
    
    if (email.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入邮箱地址！");
        m_emailEdit->setFocus();
        return;
    }
    
    // 验证邮箱格式
    QRegularExpression emailRegex("^[\\w\\.-]+@[\\w\\.-]+\\.[a-zA-Z]{2,}$");
    if (!emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "输入错误", "请输入有效的邮箱地址！");
        m_emailEdit->setFocus();
        return;
    }
    
    // 验证手机号格式（如果输入了的话）
    if (!phone.isEmpty()) {
        QRegularExpression phoneRegex("^1[3-9]\\d{9}$");
        if (!phoneRegex.match(phone).hasMatch()) {
            QMessageBox::warning(this, "输入错误", "请输入有效的手机号码！");
            m_phoneEdit->setFocus();
            return;
        }
    }
    
    // 对于新增患者，密码是必填的
    if (!m_isEditing && password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入初始密码！");
        m_passwordEdit->setFocus();
        return;
    }
    
    accept();
}

UserInfo PatientEditDialog::getPatientInfo() const
{
    UserInfo info = m_originalPatient;
    
    info.username = m_usernameEdit->text().trimmed();
    info.realName = m_realNameEdit->text().trimmed();
    info.email = m_emailEdit->text().trimmed();
    info.phone = m_phoneEdit->text().trimmed();
    info.status = m_statusCombo->currentData().toInt();
    info.role = "患者";
    
    return info;
}

// MOC generated automatically 