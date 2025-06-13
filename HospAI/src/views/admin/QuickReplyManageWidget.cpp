#include "QuickReplyManageWidget.h"
#include "../common/UIStyleManager.h"

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
    connect(m_btnAdd, &QPushButton::clicked, [this]() { QMessageBox::information(this, "提示", "添加快捷回复功能待实现"); });
    connect(m_btnEdit, &QPushButton::clicked, [this]() { QMessageBox::information(this, "提示", "编辑快捷回复功能待实现"); });
    connect(m_btnDelete, &QPushButton::clicked, [this]() { QMessageBox::information(this, "提示", "删除快捷回复功能待实现"); });
    connect(m_btnExport, &QPushButton::clicked, [this]() { QMessageBox::information(this, "提示", "导出快捷回复功能待实现"); });
    connect(m_btnImport, &QPushButton::clicked, [this]() { QMessageBox::information(this, "提示", "导入快捷回复功能待实现"); });
    connect(m_btnMoveUp, &QPushButton::clicked, [this]() { QMessageBox::information(this, "提示", "上移功能待实现"); });
    connect(m_btnMoveDown, &QPushButton::clicked, [this]() { QMessageBox::information(this, "提示", "下移功能待实现"); });
    connect(m_btnSearch, &QPushButton::clicked, this, &QuickReplyManageWidget::onSearchReplies);
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

// 其他槽函数的简化实现
void QuickReplyManageWidget::onAddReply() { QMessageBox::information(this, "提示", "功能开发中..."); }
void QuickReplyManageWidget::onEditReply() { QMessageBox::information(this, "提示", "功能开发中..."); }
void QuickReplyManageWidget::onDeleteReply() { QMessageBox::information(this, "提示", "功能开发中..."); }
void QuickReplyManageWidget::onReplySelectionChanged() { /* 暂时留空 */ }
void QuickReplyManageWidget::onExportReplies() { QMessageBox::information(this, "提示", "功能开发中..."); }
void QuickReplyManageWidget::onImportReplies() { QMessageBox::information(this, "提示", "功能开发中..."); }
void QuickReplyManageWidget::onMoveUp() { QMessageBox::information(this, "提示", "功能开发中..."); }
void QuickReplyManageWidget::onMoveDown() { QMessageBox::information(this, "提示", "功能开发中..."); }
void QuickReplyManageWidget::showReplyDialog(const QuickReply& reply) { Q_UNUSED(reply); QMessageBox::information(this, "提示", "功能开发中..."); }
void QuickReplyManageWidget::updateSortOrder() { /* 暂时留空 */ }

// ========== QuickReplyEditDialog 简化实现 ==========

QuickReplyEditDialog::QuickReplyEditDialog(const QuickReply& reply, QWidget *parent)
    : QDialog(parent)
    , m_originalReply(reply)
    , m_isEditing(reply.id > 0)
{
    setWindowTitle("快捷回复编辑对话框（开发中）");
    resize(300, 200);
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("快捷回复编辑功能正在开发中...", this));
    
    QPushButton* closeButton = new QPushButton("关闭", this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);
    layout->addWidget(closeButton);
}

void QuickReplyEditDialog::setupUI() { /* 简化实现 */ }
void QuickReplyEditDialog::onAccept() { accept(); }
void QuickReplyEditDialog::onPreview() { QMessageBox::information(this, "预览", "预览功能开发中..."); }

QuickReply QuickReplyEditDialog::getQuickReply() const
{
    return m_originalReply;
}

// MOC generated automatically
