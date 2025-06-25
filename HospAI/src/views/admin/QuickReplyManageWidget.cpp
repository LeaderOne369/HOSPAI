#include "QuickReplyManageWidget.h"
#include "../common/UIStyleManager.h"
#include <QFileDialog>
#include <QDateTime>

QuickReplyManageWidget::QuickReplyManageWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_dbManager(nullptr)
{
    setupUI();
}

void QuickReplyManageWidget::setDatabaseManager(DatabaseManager* dbManager)
{
    m_dbManager = dbManager;
    loadQuickReplies();
}

void QuickReplyManageWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    m_mainLayout->setSpacing(15);
    
    // 标题
    QLabel* titleLabel = new QLabel("快捷回复管理", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2C3E50; margin-bottom: 10px;");
    m_mainLayout->addWidget(titleLabel);
    
    // 顶部工具栏
    m_toolLayout = new QHBoxLayout;
    m_toolLayout->setSpacing(10);
    
    m_btnAdd = new QPushButton("➕ 新增回复", this);
    m_btnEdit = new QPushButton("✏️ 编辑回复", this);
    m_btnDelete = new QPushButton("🗑️ 删除回复", this);
    m_btnRefresh = new QPushButton("🔄 刷新", this);
    m_btnExport = new QPushButton("📤 导出", this);
    m_btnImport = new QPushButton("📥 导入", this);
    m_btnMoveUp = new QPushButton("⬆️ 上移", this);
    m_btnMoveDown = new QPushButton("⬇️ 下移", this);
    
    UIStyleManager::applyButtonStyle(m_btnAdd, "primary");
    UIStyleManager::applyButtonStyle(m_btnEdit, "secondary");
    UIStyleManager::applyButtonStyle(m_btnDelete, "error");
    UIStyleManager::applyButtonStyle(m_btnRefresh, "secondary");
    UIStyleManager::applyButtonStyle(m_btnExport, "secondary");
    UIStyleManager::applyButtonStyle(m_btnImport, "secondary");
    UIStyleManager::applyButtonStyle(m_btnMoveUp, "secondary");
    UIStyleManager::applyButtonStyle(m_btnMoveDown, "secondary");
    
    m_toolLayout->addWidget(m_btnAdd);
    m_toolLayout->addWidget(m_btnEdit);
    m_toolLayout->addWidget(m_btnDelete);
    m_toolLayout->addWidget(m_btnRefresh);
    m_toolLayout->addWidget(m_btnExport);
    m_toolLayout->addWidget(m_btnImport);
    m_toolLayout->addWidget(m_btnMoveUp);
    m_toolLayout->addWidget(m_btnMoveDown);
    m_toolLayout->addStretch();
    
    m_mainLayout->addLayout(m_toolLayout);
    
    // 搜索区域
    m_searchGroup = new QGroupBox("搜索条件", this);
    UIStyleManager::applyGroupBoxStyle(m_searchGroup);
    
    QHBoxLayout* searchLayout = new QHBoxLayout(m_searchGroup);
    searchLayout->setSpacing(10);
    
    searchLayout->addWidget(new QLabel("关键字:", this));
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("搜索标题或内容...");
    UIStyleManager::applyLineEditStyle(m_searchEdit);
    searchLayout->addWidget(m_searchEdit);
    
    searchLayout->addWidget(new QLabel("分类:", this));
    m_categoryFilter = new QComboBox(this);
    m_categoryFilter->addItems({"全部", "问候语", "常见问题", "结束语", "其他"});
    // UIStyleManager::applyComboBoxStyle(m_categoryFilter); // 暂时注释掉
    searchLayout->addWidget(m_categoryFilter);
    
    searchLayout->addWidget(new QLabel("状态:", this));
    m_statusFilter = new QComboBox(this);
    m_statusFilter->addItems({"全部", "启用", "禁用"});
    // UIStyleManager::applyComboBoxStyle(m_statusFilter); // 暂时注释掉
    searchLayout->addWidget(m_statusFilter);
    
    m_btnSearch = new QPushButton("🔍 搜索", this);
    UIStyleManager::applyButtonStyle(m_btnSearch, "primary");
    searchLayout->addWidget(m_btnSearch);
    
    m_mainLayout->addWidget(m_searchGroup);
    
    // 快捷回复列表表格
    m_replyTable = new QTableWidget(this);
    m_replyTable->setColumnCount(6);
    QStringList headers = {"序号", "标题", "内容预览", "分类", "状态", "更新时间"};
    m_replyTable->setHorizontalHeaderLabels(headers);
    
    // 设置表格属性
    m_replyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_replyTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_replyTable->setAlternatingRowColors(true);
    m_replyTable->setSortingEnabled(true);
    
    // UIStyleManager::applyTableStyle(m_replyTable); // 暂时注释掉
    m_mainLayout->addWidget(m_replyTable);
    
    // 统计信息
    m_statsLabel = new QLabel("共 0 条快捷回复", this);
    m_statsLabel->setStyleSheet("color: #7F8C8D; font-size: 12px;");
    m_mainLayout->addWidget(m_statsLabel);
    
    // 连接信号槽
    connect(m_btnRefresh, &QPushButton::clicked, this, &QuickReplyManageWidget::onRefreshReplies);
    connect(m_btnAdd, &QPushButton::clicked, this, &QuickReplyManageWidget::onAddReply);
    connect(m_btnEdit, &QPushButton::clicked, this, &QuickReplyManageWidget::onEditReply);
    connect(m_btnDelete, &QPushButton::clicked, this, &QuickReplyManageWidget::onDeleteReply);
    connect(m_btnExport, &QPushButton::clicked, this, &QuickReplyManageWidget::onExportReplies);
    connect(m_btnImport, &QPushButton::clicked, this, &QuickReplyManageWidget::onImportReplies);
    connect(m_btnMoveUp, &QPushButton::clicked, this, &QuickReplyManageWidget::onMoveUp);
    connect(m_btnMoveDown, &QPushButton::clicked, this, &QuickReplyManageWidget::onMoveDown);
    connect(m_btnSearch, &QPushButton::clicked, this, &QuickReplyManageWidget::onSearchReplies);
    connect(m_replyTable, &QTableWidget::itemSelectionChanged, this, &QuickReplyManageWidget::onReplySelectionChanged);
    connect(m_replyTable, &QTableWidget::itemDoubleClicked, [this]() { onEditReply(); });
}

void QuickReplyManageWidget::loadQuickReplies()
{
    if (!m_dbManager) return;
    
    // 从数据库加载真实的快捷回复数据
    m_replies = m_dbManager->getAllQuickReplies();
    
    m_replyTable->setRowCount(m_replies.size());
    
    for (int i = 0; i < m_replies.size(); ++i) {
        addReplyToTable(i, m_replies[i]);
    }
    
    // 更新统计信息
    m_statsLabel->setText(QString("共 %1 条快捷回复").arg(m_replies.size()));
}

void QuickReplyManageWidget::addReplyToTable(int row, const QuickReply& reply)
{
    m_replyTable->setItem(row, 0, new QTableWidgetItem(QString::number(reply.sortOrder)));
    m_replyTable->setItem(row, 1, new QTableWidgetItem(reply.title));
    
    QString preview = reply.content.left(30);
    if (reply.content.length() > 30) preview += "...";
    m_replyTable->setItem(row, 2, new QTableWidgetItem(preview));
    
    m_replyTable->setItem(row, 3, new QTableWidgetItem(reply.category));
    
    QString statusText = reply.isActive ? "启用" : "禁用";
    QTableWidgetItem* statusItem = new QTableWidgetItem(statusText);
    statusItem->setForeground(reply.isActive ? QColor("#27AE60") : QColor("#E74C3C"));
    m_replyTable->setItem(row, 4, statusItem);
    
    m_replyTable->setItem(row, 5, new QTableWidgetItem(reply.updatedAt.toString("yyyy-MM-dd hh:mm")));
}

void QuickReplyManageWidget::onRefreshReplies()
{
    loadQuickReplies();
}

void QuickReplyManageWidget::onSearchReplies()
{
    QString keyword = m_searchEdit->text().trimmed().toLower();
    
    for (int i = 0; i < m_replyTable->rowCount(); ++i) {
        bool show = true;
        
        if (!keyword.isEmpty()) {
            bool found = false;
            for (int j = 1; j < 4; ++j) { // 搜索标题、内容预览、分类
                QTableWidgetItem* item = m_replyTable->item(i, j);
                if (item && item->text().toLower().contains(keyword)) {
                    found = true;
                    break;
                }
            }
            if (!found) show = false;
        }
        
        m_replyTable->setRowHidden(i, !show);
    }
}

void QuickReplyManageWidget::onAddReply()
{
    showReplyDialog();
}

void QuickReplyManageWidget::onEditReply()
{
    int currentRow = m_replyTable->currentRow();
    if (currentRow < 0 || currentRow >= m_replies.size()) {
        QMessageBox::warning(this, "提示", "请选择要编辑的快捷回复");
        return;
    }
    
    showReplyDialog(m_replies[currentRow]);
}

void QuickReplyManageWidget::onDeleteReply()
{
    int currentRow = m_replyTable->currentRow();
    if (currentRow < 0 || currentRow >= m_replies.size()) {
        QMessageBox::warning(this, "提示", "请选择要删除的快捷回复");
        return;
    }
    
    const QuickReply& reply = m_replies[currentRow];
    
    int ret = QMessageBox::question(this, "确认删除", 
        QString("确定要删除快捷回复 \"%1\" 吗？").arg(reply.title),
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        if (m_dbManager && m_dbManager->deleteQuickReply(reply.id)) {
            QMessageBox::information(this, "成功", "快捷回复删除成功");
            loadQuickReplies();
        } else {
            QMessageBox::warning(this, "错误", "删除失败，请重试");
        }
    }
}

void QuickReplyManageWidget::onReplySelectionChanged()
{
    int currentRow = m_replyTable->currentRow();
    bool hasSelection = currentRow >= 0 && currentRow < m_replies.size();
    
    m_btnEdit->setEnabled(hasSelection);
    m_btnDelete->setEnabled(hasSelection);
    m_btnMoveUp->setEnabled(hasSelection && currentRow > 0);
    m_btnMoveDown->setEnabled(hasSelection && currentRow < m_replies.size() - 1);
}

void QuickReplyManageWidget::onExportReplies()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出快捷回复", 
        QString("quick_replies_%1.json").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "JSON文件 (*.json)");
    
    if (!fileName.isEmpty()) {
        // 这里可以实现导出到JSON文件的功能
        QMessageBox::information(this, "提示", "导出功能将在后续版本中实现");
    }
}

void QuickReplyManageWidget::onImportReplies()
{
    QString fileName = QFileDialog::getOpenFileName(this, "导入快捷回复", 
        "", "JSON文件 (*.json)");
    
    if (!fileName.isEmpty()) {
        // 这里可以实现从JSON文件导入的功能
        QMessageBox::information(this, "提示", "导入功能将在后续版本中实现");
    }
}

void QuickReplyManageWidget::onMoveUp()
{
    int currentRow = m_replyTable->currentRow();
    if (currentRow <= 0 || currentRow >= m_replies.size()) return;
    
    // 交换当前行和上一行的排序
    QuickReply& current = m_replies[currentRow];
    QuickReply& previous = m_replies[currentRow - 1];
    
    int tempOrder = current.sortOrder;
    current.sortOrder = previous.sortOrder;
    previous.sortOrder = tempOrder;
    
    // 更新数据库
    if (m_dbManager) {
        m_dbManager->updateQuickReply(current.id, current.title, current.content, current.category, current.sortOrder);
        m_dbManager->updateQuickReply(previous.id, previous.title, previous.content, previous.category, previous.sortOrder);
    }
    
    loadQuickReplies();
    m_replyTable->selectRow(currentRow - 1);
}

void QuickReplyManageWidget::onMoveDown()
{
    int currentRow = m_replyTable->currentRow();
    if (currentRow < 0 || currentRow >= m_replies.size() - 1) return;
    
    // 交换当前行和下一行的排序
    QuickReply& current = m_replies[currentRow];
    QuickReply& next = m_replies[currentRow + 1];
    
    int tempOrder = current.sortOrder;
    current.sortOrder = next.sortOrder;
    next.sortOrder = tempOrder;
    
    // 更新数据库
    if (m_dbManager) {
        m_dbManager->updateQuickReply(current.id, current.title, current.content, current.category, current.sortOrder);
        m_dbManager->updateQuickReply(next.id, next.title, next.content, next.category, next.sortOrder);
    }
    
    loadQuickReplies();
    m_replyTable->selectRow(currentRow + 1);
}

void QuickReplyManageWidget::showReplyDialog(const QuickReply& reply)
{
    QuickReplyEditDialog dialog(reply, this);
    if (dialog.exec() == QDialog::Accepted) {
        QuickReply newReply = dialog.getQuickReply();
        
        bool success = false;
        if (newReply.id > 0) {
            // 编辑现有回复
            success = m_dbManager && m_dbManager->updateQuickReply(
                newReply.id, newReply.title, newReply.content, 
                newReply.category, newReply.sortOrder);
        } else {
            // 添加新回复
            success = m_dbManager && m_dbManager->addQuickReply(
                newReply.title, newReply.content, newReply.category, 
                m_replies.size() + 1);
        }
        
        if (success) {
            QMessageBox::information(this, "成功", 
                newReply.id > 0 ? "快捷回复更新成功" : "快捷回复添加成功");
            loadQuickReplies();
        } else {
            QMessageBox::warning(this, "错误", "操作失败，请重试");
        }
    }
}

void QuickReplyManageWidget::updateSortOrder()
{
    // 重新排序所有快捷回复
    for (int i = 0; i < m_replies.size(); ++i) {
        m_replies[i].sortOrder = i + 1;
        if (m_dbManager) {
            m_dbManager->updateQuickReply(m_replies[i].id, m_replies[i].title, 
                m_replies[i].content, m_replies[i].category, m_replies[i].sortOrder);
        }
    }
}

// ========== QuickReplyEditDialog 完整实现 ==========

QuickReplyEditDialog::QuickReplyEditDialog(const QuickReply& reply, QWidget *parent)
    : QDialog(parent)
    , m_originalReply(reply)
    , m_isEditing(reply.id > 0)
{
    setWindowTitle(m_isEditing ? "编辑快捷回复" : "添加快捷回复");
    resize(500, 400);
    setupUI();
    
    if (m_isEditing) {
        // 填充现有数据
        m_titleEdit->setText(reply.title);
        m_contentEdit->setPlainText(reply.content);
        m_categoryCombo->setCurrentText(reply.category);
        m_sortSpin->setValue(reply.sortOrder);
        m_statusCombo->setCurrentIndex(reply.isActive ? 0 : 1);
    }
}

void QuickReplyEditDialog::setupUI()
{
    m_formLayout = new QFormLayout(this);
    m_formLayout->setSpacing(12);
    
    // 标题输入
    m_titleEdit = new QLineEdit(this);
    m_titleEdit->setPlaceholderText("请输入快捷回复标题...");
    m_formLayout->addRow("标题:", m_titleEdit);
    
    // 内容输入
    m_contentEdit = new QTextEdit(this);
    m_contentEdit->setPlaceholderText("请输入快捷回复内容...");
    m_contentEdit->setMaximumHeight(150);
    m_formLayout->addRow("内容:", m_contentEdit);
    
    // 分类选择
    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->addItems({"问候语", "常见问题", "结束语", "其他"});
    m_formLayout->addRow("分类:", m_categoryCombo);
    
    // 排序
    m_sortSpin = new QSpinBox(this);
    m_sortSpin->setRange(1, 999);
    m_sortSpin->setValue(1);
    m_formLayout->addRow("排序:", m_sortSpin);
    
    // 状态
    m_statusCombo = new QComboBox(this);
    m_statusCombo->addItems({"启用", "禁用"});
    m_formLayout->addRow("状态:", m_statusCombo);
    
    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    
    m_previewButton = new QPushButton("预览", this);
    m_okButton = new QPushButton("确定", this);
    m_cancelButton = new QPushButton("取消", this);
    
    m_okButton->setDefault(true);
    
    buttonLayout->addWidget(m_previewButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    
    m_formLayout->addRow(buttonLayout);
    
    // 连接信号
    connect(m_previewButton, &QPushButton::clicked, this, &QuickReplyEditDialog::onPreview);
    connect(m_okButton, &QPushButton::clicked, this, &QuickReplyEditDialog::onAccept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void QuickReplyEditDialog::onAccept()
{
    QString title = m_titleEdit->text().trimmed();
    QString content = m_contentEdit->toPlainText().trimmed();
    
    if (title.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入标题");
        m_titleEdit->setFocus();
        return;
    }
    
    if (content.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入内容");
        m_contentEdit->setFocus();
        return;
    }
    
    accept();
}

void QuickReplyEditDialog::onPreview()
{
    QString content = m_contentEdit->toPlainText();
    if (content.isEmpty()) {
        QMessageBox::information(this, "预览", "内容为空");
        return;
    }
    
    QMessageBox previewBox(this);
    previewBox.setWindowTitle("快捷回复预览");
    previewBox.setText(QString("标题: %1\n\n内容预览:").arg(m_titleEdit->text()));
    previewBox.setDetailedText(content);
    previewBox.setStandardButtons(QMessageBox::Ok);
    previewBox.exec();
}

QuickReply QuickReplyEditDialog::getQuickReply() const
{
    QuickReply reply = m_originalReply;
    
    reply.title = m_titleEdit->text().trimmed();
    reply.content = m_contentEdit->toPlainText().trimmed();
    reply.category = m_categoryCombo->currentText();
    reply.sortOrder = m_sortSpin->value();
    reply.isActive = (m_statusCombo->currentIndex() == 0);
    reply.updatedAt = QDateTime::currentDateTime();
    
    if (!m_isEditing) {
        reply.createdAt = QDateTime::currentDateTime();
    }
    
    return reply;
}

// MOC generated automatically
