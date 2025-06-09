#include "SystemConfigWidget.h"
#include "../common/UIStyleManager.h"
#include <QHeaderView>
#include <QMessageBox>

SystemConfigWidget::SystemConfigWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    loadConfig();
}

void SystemConfigWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(15);
    
    // 标题
    QLabel* titleLabel = new QLabel("系统设置");
    UIStyleManager::applyLabelStyle(titleLabel, "title");
    m_mainLayout->addWidget(titleLabel);
    
    // 顶部按钮
    m_buttonLayout = new QHBoxLayout;
    
    m_btnSave = new QPushButton("💾 保存配置");
    m_btnReset = new QPushButton("🔄 重置");
    m_btnImport = new QPushButton("📁 导入");
    m_btnExport = new QPushButton("📤 导出");
    
    UIStyleManager::applyButtonStyle(m_btnSave, "success");
    UIStyleManager::applyButtonStyle(m_btnReset, "warning");
    UIStyleManager::applyButtonStyle(m_btnImport, "secondary");
    UIStyleManager::applyButtonStyle(m_btnExport, "secondary");
    
    connect(m_btnSave, &QPushButton::clicked, this, &SystemConfigWidget::onSaveConfig);
    connect(m_btnReset, &QPushButton::clicked, this, &SystemConfigWidget::onResetConfig);
    connect(m_btnImport, &QPushButton::clicked, this, &SystemConfigWidget::onImportConfig);
    connect(m_btnExport, &QPushButton::clicked, this, &SystemConfigWidget::onExportConfig);
    
    m_buttonLayout->addWidget(m_btnSave);
    m_buttonLayout->addWidget(m_btnReset);
    m_buttonLayout->addWidget(m_btnImport);
    m_buttonLayout->addWidget(m_btnExport);
    m_buttonLayout->addStretch();
    
    m_mainLayout->addLayout(m_buttonLayout);
    
    // 选项卡
    m_tabWidget = new QTabWidget;
    
    setupGeneralTab();
    setupAIConfigTab();
    setupFAQTab();
    setupDepartmentTab();
    
    m_tabWidget->addTab(m_generalTab, "⚙️ 常规设置");
    m_tabWidget->addTab(m_aiConfigTab, "🤖 AI配置");
    m_tabWidget->addTab(m_faqTab, "❓ FAQ管理");
    m_tabWidget->addTab(m_departmentTab, "🏥 科室管理");
    
    m_mainLayout->addWidget(m_tabWidget);
}

void SystemConfigWidget::setupGeneralTab()
{
    m_generalTab = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(m_generalTab);
    
    // 系统基本信息
    QGroupBox* basicGroup = new QGroupBox("系统基本信息");
    UIStyleManager::applyGroupBoxStyle(basicGroup);
    QGridLayout* basicLayout = new QGridLayout(basicGroup);
    
    basicLayout->addWidget(new QLabel("系统名称:"), 0, 0);
    m_systemName = new QLineEdit("医院智慧客服系统");
    UIStyleManager::applyLineEditStyle(m_systemName);
    basicLayout->addWidget(m_systemName, 0, 1);
    
    basicLayout->addWidget(new QLabel("系统版本:"), 1, 0);
    m_systemVersion = new QLineEdit("1.0.0");
    UIStyleManager::applyLineEditStyle(m_systemVersion);
    basicLayout->addWidget(m_systemVersion, 1, 1);
    
    basicLayout->addWidget(new QLabel("最大用户数:"), 2, 0);
    m_maxUsers = new QSpinBox;
    m_maxUsers->setRange(1, 10000);
    m_maxUsers->setValue(1000);
    basicLayout->addWidget(m_maxUsers, 2, 1);
    
    basicLayout->addWidget(new QLabel("会话超时(分钟):"), 3, 0);
    m_sessionTimeout = new QSpinBox;
    m_sessionTimeout->setRange(5, 120);
    m_sessionTimeout->setValue(30);
    basicLayout->addWidget(m_sessionTimeout, 3, 1);
    
    layout->addWidget(basicGroup);
    
    // 系统选项
    QGroupBox* optionGroup = new QGroupBox("系统选项");
    UIStyleManager::applyGroupBoxStyle(optionGroup);
    QVBoxLayout* optionLayout = new QVBoxLayout(optionGroup);
    
    m_enableLogging = new QCheckBox("启用系统日志");
    m_enableLogging->setChecked(true);
    optionLayout->addWidget(m_enableLogging);
    
    m_enableBackup = new QCheckBox("启用自动备份");
    m_enableBackup->setChecked(true);
    optionLayout->addWidget(m_enableBackup);
    
    QHBoxLayout* logLayout = new QHBoxLayout;
    logLayout->addWidget(new QLabel("日志级别:"));
    m_logLevel = new QComboBox;
    m_logLevel->addItems({"Debug", "Info", "Warning", "Error"});
    m_logLevel->setCurrentText("Info");
    logLayout->addWidget(m_logLevel);
    logLayout->addStretch();
    optionLayout->addLayout(logLayout);
    
    layout->addWidget(optionGroup);
    layout->addStretch();
}

void SystemConfigWidget::setupAIConfigTab()
{
    m_aiConfigTab = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(m_aiConfigTab);
    
    QGroupBox* aiGroup = new QGroupBox("AI智能配置");
    UIStyleManager::applyGroupBoxStyle(aiGroup);
    QVBoxLayout* aiLayout = new QVBoxLayout(aiGroup);
    
    QLabel* confLabel = new QLabel("AI置信度阈值: 75%");
    m_aiConfidence = new QSlider(Qt::Horizontal);
    m_aiConfidence->setRange(0, 100);
    m_aiConfidence->setValue(75);
    connect(m_aiConfidence, &QSlider::valueChanged, [confLabel](int value) {
        confLabel->setText(QString("AI置信度阈值: %1%").arg(value));
    });
    aiLayout->addWidget(confLabel);
    aiLayout->addWidget(m_aiConfidence);
    
    layout->addWidget(aiGroup);
    layout->addStretch();
}

void SystemConfigWidget::setupFAQTab()
{
    m_faqTab = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(m_faqTab);
    
    // 工具栏
    QHBoxLayout* toolLayout = new QHBoxLayout;
    m_btnAddFAQ = new QPushButton("➕ 添加FAQ");
    m_btnEditFAQ = new QPushButton("✏️ 编辑");
    m_btnDeleteFAQ = new QPushButton("🗑️ 删除");
    
    UIStyleManager::applyButtonStyle(m_btnAddFAQ, "success");
    UIStyleManager::applyButtonStyle(m_btnEditFAQ, "primary");
    UIStyleManager::applyButtonStyle(m_btnDeleteFAQ, "error");
    
    connect(m_btnAddFAQ, &QPushButton::clicked, this, &SystemConfigWidget::onAddFAQ);
    connect(m_btnEditFAQ, &QPushButton::clicked, this, &SystemConfigWidget::onEditFAQ);
    connect(m_btnDeleteFAQ, &QPushButton::clicked, this, &SystemConfigWidget::onDeleteFAQ);
    
    toolLayout->addWidget(m_btnAddFAQ);
    toolLayout->addWidget(m_btnEditFAQ);
    toolLayout->addWidget(m_btnDeleteFAQ);
    toolLayout->addStretch();
    
    layout->addLayout(toolLayout);
    
    // FAQ表格
    m_faqTable = new QTableWidget;
    m_faqTable->setColumnCount(3);
    QStringList headers = {"问题", "答案", "分类"};
    m_faqTable->setHorizontalHeaderLabels(headers);
    
    // 添加示例数据
    m_faqTable->setRowCount(3);
    m_faqTable->setItem(0, 0, new QTableWidgetItem("如何挂号？"));
    m_faqTable->setItem(0, 1, new QTableWidgetItem("您可以通过网上预约或现场挂号"));
    m_faqTable->setItem(0, 2, new QTableWidgetItem("挂号"));
    
    m_faqTable->setItem(1, 0, new QTableWidgetItem("门诊时间是什么？"));
    m_faqTable->setItem(1, 1, new QTableWidgetItem("门诊时间：周一至周日 8:00-17:00"));
    m_faqTable->setItem(1, 2, new QTableWidgetItem("门诊"));
    
    m_faqTable->setItem(2, 0, new QTableWidgetItem("如何查看检查结果？"));
    m_faqTable->setItem(2, 1, new QTableWidgetItem("可通过手机APP或网站查询"));
    m_faqTable->setItem(2, 2, new QTableWidgetItem("检查"));
    
    m_faqTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(m_faqTable);
}

void SystemConfigWidget::setupDepartmentTab()
{
    m_departmentTab = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(m_departmentTab);
    
    // 工具栏
    QHBoxLayout* toolLayout = new QHBoxLayout;
    m_btnAddDept = new QPushButton("➕ 添加科室");
    m_btnEditDept = new QPushButton("✏️ 编辑");
    m_btnDeleteDept = new QPushButton("🗑️ 删除");
    
    UIStyleManager::applyButtonStyle(m_btnAddDept, "success");
    UIStyleManager::applyButtonStyle(m_btnEditDept, "primary");
    UIStyleManager::applyButtonStyle(m_btnDeleteDept, "error");
    
    connect(m_btnAddDept, &QPushButton::clicked, this, &SystemConfigWidget::onAddDepartment);
    connect(m_btnEditDept, &QPushButton::clicked, this, &SystemConfigWidget::onEditDepartment);
    connect(m_btnDeleteDept, &QPushButton::clicked, this, &SystemConfigWidget::onDeleteDepartment);
    
    toolLayout->addWidget(m_btnAddDept);
    toolLayout->addWidget(m_btnEditDept);
    toolLayout->addWidget(m_btnDeleteDept);
    toolLayout->addStretch();
    
    layout->addLayout(toolLayout);
    
    // 科室表格
    m_departmentTable = new QTableWidget;
    m_departmentTable->setColumnCount(4);
    QStringList headers = {"科室名称", "位置", "电话", "描述"};
    m_departmentTable->setHorizontalHeaderLabels(headers);
    
    // 添加示例数据
    m_departmentTable->setRowCount(3);
    m_departmentTable->setItem(0, 0, new QTableWidgetItem("内科"));
    m_departmentTable->setItem(0, 1, new QTableWidgetItem("1楼东侧"));
    m_departmentTable->setItem(0, 2, new QTableWidgetItem("010-12345678"));
    m_departmentTable->setItem(0, 3, new QTableWidgetItem("内科疾病诊疗"));
    
    m_departmentTable->setItem(1, 0, new QTableWidgetItem("外科"));
    m_departmentTable->setItem(1, 1, new QTableWidgetItem("2楼西侧"));
    m_departmentTable->setItem(1, 2, new QTableWidgetItem("010-87654321"));
    m_departmentTable->setItem(1, 3, new QTableWidgetItem("外科手术治疗"));
    
    m_departmentTable->setItem(2, 0, new QTableWidgetItem("儿科"));
    m_departmentTable->setItem(2, 1, new QTableWidgetItem("3楼南侧"));
    m_departmentTable->setItem(2, 2, new QTableWidgetItem("010-11223344"));
    m_departmentTable->setItem(2, 3, new QTableWidgetItem("儿童疾病诊疗"));
    
    m_departmentTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(m_departmentTable);
}

void SystemConfigWidget::loadConfig()
{
    // 从配置文件或数据库加载配置
}

void SystemConfigWidget::saveConfig()
{
    // 保存配置到文件或数据库
}

void SystemConfigWidget::onSaveConfig()
{
    saveConfig();
    QMessageBox::information(this, "保存成功", "系统配置已保存！");
}

void SystemConfigWidget::onResetConfig()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "重置配置",
        "确定要重置所有配置到默认值吗？",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        loadConfig();
        QMessageBox::information(this, "重置完成", "配置已重置到默认值！");
    }
}

void SystemConfigWidget::onImportConfig()
{
    QMessageBox::information(this, "导入配置", "配置导入功能将在后续版本中实现");
}

void SystemConfigWidget::onExportConfig()
{
    QMessageBox::information(this, "导出配置", "配置导出功能将在后续版本中实现");
}

void SystemConfigWidget::onAddFAQ()
{
    QMessageBox::information(this, "添加FAQ", "FAQ添加功能将在后续版本中实现");
}

void SystemConfigWidget::onEditFAQ()
{
    QMessageBox::information(this, "编辑FAQ", "FAQ编辑功能将在后续版本中实现");
}

void SystemConfigWidget::onDeleteFAQ()
{
    QMessageBox::information(this, "删除FAQ", "FAQ删除功能将在后续版本中实现");
}

void SystemConfigWidget::onAddDepartment()
{
    QMessageBox::information(this, "添加科室", "科室添加功能将在后续版本中实现");
}

void SystemConfigWidget::onEditDepartment()
{
    QMessageBox::information(this, "编辑科室", "科室编辑功能将在后续版本中实现");
}

void SystemConfigWidget::onDeleteDepartment()
{
    QMessageBox::information(this, "删除科室", "科室删除功能将在后续版本中实现");
}

// FAQEditDialog 实现
FAQEditDialog::FAQEditDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("编辑FAQ");
    setFixedSize(400, 300);
    setModal(true);
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);
    
    // 问题输入
    QLabel* questionLabel = new QLabel("问题:");
    m_editQuestion = new QLineEdit;
    m_editQuestion->setPlaceholderText("输入常见问题...");
    
    // 答案输入
    QLabel* answerLabel = new QLabel("答案:");
    m_editAnswer = new QTextEdit;
    m_editAnswer->setPlaceholderText("输入问题答案...");
    m_editAnswer->setMaximumHeight(120);
    
    // 分类选择
    QLabel* categoryLabel = new QLabel("分类:");
    m_comboCategory = new QComboBox;
    m_comboCategory->addItems({"挂号", "门诊", "检查", "其他"});
    
    // 按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    QPushButton* okButton = new QPushButton("确定");
    QPushButton* cancelButton = new QPushButton("取消");
    
    UIStyleManager::applyButtonStyle(okButton, "primary");
    UIStyleManager::applyButtonStyle(cancelButton, "secondary");
    
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    
    layout->addWidget(questionLabel);
    layout->addWidget(m_editQuestion);
    layout->addWidget(answerLabel);
    layout->addWidget(m_editAnswer);
    layout->addWidget(categoryLabel);
    layout->addWidget(m_comboCategory);
    layout->addLayout(buttonLayout);
}

QString FAQEditDialog::getQuestion() const
{
    return m_editQuestion->text();
}

QString FAQEditDialog::getAnswer() const
{
    return m_editAnswer->toPlainText();
}

QString FAQEditDialog::getCategory() const
{
    return m_comboCategory->currentText();
}

void FAQEditDialog::setFAQData(const QString& question, const QString& answer, const QString& category)
{
    m_editQuestion->setText(question);
    m_editAnswer->setPlainText(answer);
    m_comboCategory->setCurrentText(category);
}

// DepartmentEditDialog 实现
DepartmentEditDialog::DepartmentEditDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("编辑科室");
    setFixedSize(400, 250);
    setModal(true);
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);
    
    // 科室名称
    QLabel* nameLabel = new QLabel("科室名称:");
    m_editName = new QLineEdit;
    m_editName->setPlaceholderText("输入科室名称...");
    
    // 位置
    QLabel* locationLabel = new QLabel("位置:");
    m_editLocation = new QLineEdit;
    m_editLocation->setPlaceholderText("输入科室位置...");
    
    // 电话
    QLabel* phoneLabel = new QLabel("电话:");
    m_editPhone = new QLineEdit;
    m_editPhone->setPlaceholderText("输入联系电话...");
    
    // 描述
    QLabel* descLabel = new QLabel("描述:");
    m_editDescription = new QTextEdit;
    m_editDescription->setPlaceholderText("输入科室描述...");
    m_editDescription->setMaximumHeight(80);
    
    // 按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    QPushButton* okButton = new QPushButton("确定");
    QPushButton* cancelButton = new QPushButton("取消");
    
    UIStyleManager::applyButtonStyle(okButton, "primary");
    UIStyleManager::applyButtonStyle(cancelButton, "secondary");
    
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    
    layout->addWidget(nameLabel);
    layout->addWidget(m_editName);
    layout->addWidget(locationLabel);
    layout->addWidget(m_editLocation);
    layout->addWidget(phoneLabel);
    layout->addWidget(m_editPhone);
    layout->addWidget(descLabel);
    layout->addWidget(m_editDescription);
    layout->addLayout(buttonLayout);
}

QString DepartmentEditDialog::getName() const
{
    return m_editName->text();
}

QString DepartmentEditDialog::getLocation() const
{
    return m_editLocation->text();
}

QString DepartmentEditDialog::getPhone() const
{
    return m_editPhone->text();
}

QString DepartmentEditDialog::getDescription() const
{
    return m_editDescription->toPlainText();
}

void DepartmentEditDialog::setDepartmentData(const QString& name, const QString& location, 
                                            const QString& phone, const QString& description)
{
    m_editName->setText(name);
    m_editLocation->setText(location);
    m_editPhone->setText(phone);
    m_editDescription->setPlainText(description);
} 