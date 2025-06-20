#include "SystemConfigWidget.h"
#include "../common/UIStyleManager.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QDateTime>

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
    FAQEditDialog dialog(this);
    
    connect(&dialog, &FAQEditDialog::faqSaved, this, [this](const FAQItem& item) {
        // 添加到表格
        int row = m_faqTable->rowCount();
        m_faqTable->insertRow(row);
        m_faqTable->setItem(row, 0, new QTableWidgetItem(item.question));
        m_faqTable->setItem(row, 1, new QTableWidgetItem(item.answer));
        m_faqTable->setItem(row, 2, new QTableWidgetItem(item.category));
        
        QMessageBox::information(this, "添加成功", "FAQ已成功添加！");
    });
    
    dialog.exec();
}

void SystemConfigWidget::onEditFAQ()
{
    int currentRow = m_faqTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "选择错误", "请先选择要编辑的FAQ！");
        return;
    }
    
    // 构造当前FAQ项
    FAQItem item;
    item.id = currentRow; // 简化处理，使用行号作为ID
    item.question = m_faqTable->item(currentRow, 0)->text();
    item.answer = m_faqTable->item(currentRow, 1)->text();
    item.category = m_faqTable->item(currentRow, 2)->text();
    item.priority = 1;
    item.enabled = true;
    
    FAQEditDialog dialog(item, this);
    
    connect(&dialog, &FAQEditDialog::faqSaved, this, [this, currentRow](const FAQItem& item) {
        // 更新表格
        m_faqTable->setItem(currentRow, 0, new QTableWidgetItem(item.question));
        m_faqTable->setItem(currentRow, 1, new QTableWidgetItem(item.answer));
        m_faqTable->setItem(currentRow, 2, new QTableWidgetItem(item.category));
        
        QMessageBox::information(this, "更新成功", "FAQ已成功更新！");
    });
    
    dialog.exec();
}

void SystemConfigWidget::onDeleteFAQ()
{
    int currentRow = m_faqTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "选择错误", "请先选择要删除的FAQ！");
        return;
    }
    
    QString question = m_faqTable->item(currentRow, 0)->text();
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认删除",
        QString("确定要删除FAQ：\n\"%1\"").arg(question),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        m_faqTable->removeRow(currentRow);
        QMessageBox::information(this, "删除成功", "FAQ已成功删除！");
    }
}

void SystemConfigWidget::onAddDepartment()
{
    DepartmentEditDialog dialog(this);
    
    connect(&dialog, &DepartmentEditDialog::departmentSaved, this, [this](const DepartmentEditDialog::DepartmentInfo& dept) {
        // 添加到表格
        int row = m_departmentTable->rowCount();
        m_departmentTable->insertRow(row);
        m_departmentTable->setItem(row, 0, new QTableWidgetItem(dept.name));
        m_departmentTable->setItem(row, 1, new QTableWidgetItem(dept.location));
        m_departmentTable->setItem(row, 2, new QTableWidgetItem(dept.phone));
        m_departmentTable->setItem(row, 3, new QTableWidgetItem(dept.description));
        
        QMessageBox::information(this, "添加成功", "科室已成功添加！");
    });
    
    dialog.exec();
}

void SystemConfigWidget::onEditDepartment()
{
    int currentRow = m_departmentTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "选择错误", "请先选择要编辑的科室！");
        return;
    }
    
    // 构造当前科室信息
    DepartmentEditDialog::DepartmentInfo dept;
    dept.id = currentRow; // 简化处理，使用行号作为ID
    dept.name = m_departmentTable->item(currentRow, 0)->text();
    dept.location = m_departmentTable->item(currentRow, 1)->text();
    dept.phone = m_departmentTable->item(currentRow, 2)->text();
    dept.description = m_departmentTable->item(currentRow, 3)->text();
    dept.workTime = "周一至周五 8:00-17:00";
    dept.director = "";
    dept.isActive = true;
    
    DepartmentEditDialog dialog(dept, this);
    
    connect(&dialog, &DepartmentEditDialog::departmentSaved, this, [this, currentRow](const DepartmentEditDialog::DepartmentInfo& dept) {
        // 更新表格
        m_departmentTable->setItem(currentRow, 0, new QTableWidgetItem(dept.name));
        m_departmentTable->setItem(currentRow, 1, new QTableWidgetItem(dept.location));
        m_departmentTable->setItem(currentRow, 2, new QTableWidgetItem(dept.phone));
        m_departmentTable->setItem(currentRow, 3, new QTableWidgetItem(dept.description));
        
        QMessageBox::information(this, "更新成功", "科室信息已成功更新！");
    });
    
    dialog.exec();
}

void SystemConfigWidget::onDeleteDepartment()
{
    int currentRow = m_departmentTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "选择错误", "请先选择要删除的科室！");
        return;
    }
    
    QString deptName = m_departmentTable->item(currentRow, 0)->text();
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认删除",
        QString("确定要删除科室：\n\"%1\"").arg(deptName),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        m_departmentTable->removeRow(currentRow);
        QMessageBox::information(this, "删除成功", "科室已成功删除！");
    }
}

// FAQEditDialog 实现
FAQEditDialog::FAQEditDialog(QWidget *parent)
    : QDialog(parent)
    , m_isEditMode(false)
{
    setupUI();
    connectSignals();
    
    // 初始化新FAQ项
    m_currentItem.id = -1;
    m_currentItem.priority = 1;
    m_currentItem.enabled = true;
    m_currentItem.createTime = QDateTime::currentDateTime().toString(Qt::ISODate);
}

FAQEditDialog::FAQEditDialog(const FAQItem& item, QWidget *parent)
    : QDialog(parent)
    , m_currentItem(item)
    , m_isEditMode(true)
{
    setupUI();
    connectSignals();
    setFAQItem(item);
}

void FAQEditDialog::setupUI()
{
    setWindowTitle(m_isEditMode ? "编辑FAQ" : "添加FAQ");
    setFixedSize(500, 400);
    setModal(true);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    
    // 创建表单布局
    QGridLayout* formLayout = new QGridLayout;
    formLayout->setSpacing(10);
    
    // 问题输入
    formLayout->addWidget(new QLabel("问题*:"), 0, 0);
    m_editQuestion = new QLineEdit;
    m_editQuestion->setPlaceholderText("请输入常见问题...");
    UIStyleManager::applyLineEditStyle(m_editQuestion);
    formLayout->addWidget(m_editQuestion, 0, 1);
    
    // 答案输入
    formLayout->addWidget(new QLabel("答案*:"), 1, 0);
    m_editAnswer = new QTextEdit;
    m_editAnswer->setPlaceholderText("请输入详细的答案...");
    m_editAnswer->setMaximumHeight(120);
    formLayout->addWidget(m_editAnswer, 1, 1);
    
    // 分类选择
    formLayout->addWidget(new QLabel("分类:"), 2, 0);
    m_comboCategory = new QComboBox;
    m_comboCategory->addItems({"挂号", "门诊", "检查", "住院", "费用", "其他"});
    formLayout->addWidget(m_comboCategory, 2, 1);
    
    // 关键词
    formLayout->addWidget(new QLabel("关键词:"), 3, 0);
    m_editKeywords = new QLineEdit;
    m_editKeywords->setPlaceholderText("用逗号分隔多个关键词...");
    UIStyleManager::applyLineEditStyle(m_editKeywords);
    formLayout->addWidget(m_editKeywords, 3, 1);
    
    // 优先级
    formLayout->addWidget(new QLabel("优先级:"), 4, 0);
    m_spinPriority = new QSpinBox;
    m_spinPriority->setRange(1, 10);
    m_spinPriority->setValue(1);
    m_spinPriority->setToolTip("1-10，数字越大优先级越高");
    formLayout->addWidget(m_spinPriority, 4, 1);
    
    // 启用状态
    m_checkEnabled = new QCheckBox("启用此FAQ");
    m_checkEnabled->setChecked(true);
    formLayout->addWidget(m_checkEnabled, 5, 1);
    
    mainLayout->addLayout(formLayout);
    
    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    
    m_btnPreview = new QPushButton("预览");
    m_btnCancel = new QPushButton("取消");
    m_btnOk = new QPushButton(m_isEditMode ? "更新" : "添加");
    
    UIStyleManager::applyButtonStyle(m_btnPreview, "secondary");
    UIStyleManager::applyButtonStyle(m_btnCancel, "secondary");
    UIStyleManager::applyButtonStyle(m_btnOk, "success");
    
    buttonLayout->addWidget(m_btnPreview);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_btnCancel);
    buttonLayout->addWidget(m_btnOk);
    
    mainLayout->addLayout(buttonLayout);
}

void FAQEditDialog::connectSignals()
{
    connect(m_btnOk, &QPushButton::clicked, this, &FAQEditDialog::onOkClicked);
    connect(m_btnCancel, &QPushButton::clicked, this, &FAQEditDialog::onCancelClicked);
    connect(m_btnPreview, &QPushButton::clicked, this, &FAQEditDialog::onPreviewClicked);
}

void FAQEditDialog::setFAQItem(const FAQItem& item)
{
    m_currentItem = item;
    
    m_editQuestion->setText(item.question);
    m_editAnswer->setText(item.answer);
    m_comboCategory->setCurrentText(item.category);
    m_editKeywords->setText(item.keywords);
    m_spinPriority->setValue(item.priority);
    m_checkEnabled->setChecked(item.enabled);
}

FAQItem FAQEditDialog::getFAQItem() const
{
    FAQItem item = m_currentItem;
    
    item.question = m_editQuestion->text();
    item.answer = m_editAnswer->toPlainText();
    item.category = m_comboCategory->currentText();
    item.keywords = m_editKeywords->text();
    item.priority = m_spinPriority->value();
    item.enabled = m_checkEnabled->isChecked();
    
    if (!m_isEditMode) {
        item.createTime = QDateTime::currentDateTime().toString(Qt::ISODate);
    }
    item.updateTime = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    return item;
}

bool FAQEditDialog::validateInput()
{
    if (m_editQuestion->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入问题内容！");
        m_editQuestion->setFocus();
        return false;
    }
    
    if (m_editAnswer->toPlainText().trimmed().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入答案内容！");
        m_editAnswer->setFocus();
        return false;
    }
    
    return true;
}

void FAQEditDialog::onOkClicked()
{
    if (validateInput()) {
        FAQItem item = getFAQItem();
        emit faqSaved(item);
        accept();
    }
}

void FAQEditDialog::onCancelClicked()
{
    reject();
}

void FAQEditDialog::onPreviewClicked()
{
    QString previewText = QString("问题：%1\n\n答案：%2\n\n分类：%3\n关键词：%4\n优先级：%5\n状态：%6")
                            .arg(m_editQuestion->text())
                            .arg(m_editAnswer->toPlainText())
                            .arg(m_comboCategory->currentText())
                            .arg(m_editKeywords->text())
                            .arg(m_spinPriority->value())
                            .arg(m_checkEnabled->isChecked() ? "启用" : "禁用");
    
    QMessageBox::information(this, "FAQ预览", previewText);
}

// DepartmentEditDialog 实现
DepartmentEditDialog::DepartmentEditDialog(QWidget *parent)
    : QDialog(parent)
    , m_isEditMode(false)
{
    setupUI();
    connectSignals();
    
    // 初始化新部门信息
    m_currentDept.id = -1;
    m_currentDept.isActive = true;
}

DepartmentEditDialog::DepartmentEditDialog(const DepartmentInfo& dept, QWidget *parent)
    : QDialog(parent)
    , m_currentDept(dept)
    , m_isEditMode(true)
{
    setupUI();
    connectSignals();
    setDepartmentInfo(dept);
}

void DepartmentEditDialog::setupUI()
{
    setWindowTitle(m_isEditMode ? "编辑科室" : "添加科室");
    setFixedSize(450, 350);
    setModal(true);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    
    // 创建表单布局
    QGridLayout* formLayout = new QGridLayout;
    formLayout->setSpacing(10);
    
    // 科室名称
    formLayout->addWidget(new QLabel("科室名称*:"), 0, 0);
    m_editName = new QLineEdit;
    m_editName->setPlaceholderText("请输入科室名称...");
    UIStyleManager::applyLineEditStyle(m_editName);
    formLayout->addWidget(m_editName, 0, 1);
    
    // 位置
    formLayout->addWidget(new QLabel("位置:"), 1, 0);
    m_editLocation = new QLineEdit;
    m_editLocation->setPlaceholderText("如：1楼东侧...");
    UIStyleManager::applyLineEditStyle(m_editLocation);
    formLayout->addWidget(m_editLocation, 1, 1);
    
    // 电话
    formLayout->addWidget(new QLabel("联系电话:"), 2, 0);
    m_editPhone = new QLineEdit;
    m_editPhone->setPlaceholderText("请输入联系电话...");
    UIStyleManager::applyLineEditStyle(m_editPhone);
    formLayout->addWidget(m_editPhone, 2, 1);
    
    // 工作时间
    formLayout->addWidget(new QLabel("工作时间:"), 3, 0);
    m_editWorkTime = new QLineEdit;
    m_editWorkTime->setPlaceholderText("如：周一至周五 8:00-17:00");
    m_editWorkTime->setText("周一至周五 8:00-17:00");
    UIStyleManager::applyLineEditStyle(m_editWorkTime);
    formLayout->addWidget(m_editWorkTime, 3, 1);
    
    // 科室主任
    formLayout->addWidget(new QLabel("科室主任:"), 4, 0);
    m_editDirector = new QLineEdit;
    m_editDirector->setPlaceholderText("请输入科室主任姓名...");
    UIStyleManager::applyLineEditStyle(m_editDirector);
    formLayout->addWidget(m_editDirector, 4, 1);
    
    // 描述
    formLayout->addWidget(new QLabel("科室描述:"), 5, 0);
    m_editDescription = new QTextEdit;
    m_editDescription->setPlaceholderText("请输入科室简介...");
    m_editDescription->setMaximumHeight(80);
    formLayout->addWidget(m_editDescription, 5, 1);
    
    // 启用状态
    m_checkActive = new QCheckBox("科室开放");
    m_checkActive->setChecked(true);
    formLayout->addWidget(m_checkActive, 6, 1);
    
    mainLayout->addLayout(formLayout);
    
    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    
    m_btnCancel = new QPushButton("取消");
    m_btnOk = new QPushButton(m_isEditMode ? "更新" : "添加");
    
    UIStyleManager::applyButtonStyle(m_btnCancel, "secondary");
    UIStyleManager::applyButtonStyle(m_btnOk, "success");
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_btnCancel);
    buttonLayout->addWidget(m_btnOk);
    
    mainLayout->addLayout(buttonLayout);
}

void DepartmentEditDialog::connectSignals()
{
    connect(m_btnOk, &QPushButton::clicked, this, &DepartmentEditDialog::onOkClicked);
    connect(m_btnCancel, &QPushButton::clicked, this, &DepartmentEditDialog::onCancelClicked);
}

void DepartmentEditDialog::setDepartmentInfo(const DepartmentInfo& dept)
{
    m_currentDept = dept;
    
    m_editName->setText(dept.name);
    m_editLocation->setText(dept.location);
    m_editPhone->setText(dept.phone);
    m_editDescription->setText(dept.description);
    m_editWorkTime->setText(dept.workTime);
    m_editDirector->setText(dept.director);
    m_checkActive->setChecked(dept.isActive);
}

DepartmentEditDialog::DepartmentInfo DepartmentEditDialog::getDepartmentInfo() const
{
    DepartmentInfo dept = m_currentDept;
    
    dept.name = m_editName->text();
    dept.location = m_editLocation->text();
    dept.phone = m_editPhone->text();
    dept.description = m_editDescription->toPlainText();
    dept.workTime = m_editWorkTime->text();
    dept.director = m_editDirector->text();
    dept.isActive = m_checkActive->isChecked();
    
    return dept;
}

bool DepartmentEditDialog::validateInput()
{
    if (m_editName->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入科室名称！");
        m_editName->setFocus();
        return false;
    }
    
    return true;
}

void DepartmentEditDialog::onOkClicked()
{
    if (validateInput()) {
        DepartmentInfo dept = getDepartmentInfo();
        emit departmentSaved(dept);
        accept();
    }
}

void DepartmentEditDialog::onCancelClicked()
{
    reject();
} 