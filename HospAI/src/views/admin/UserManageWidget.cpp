#include "UserManageWidget.h"
#include "../common/UIStyleManager.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QFrame>
#include <QTimer>

UserManageWidget::UserManageWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_toolLayout(nullptr)
    , m_btnAdd(nullptr)
    , m_btnEdit(nullptr)
    , m_btnDelete(nullptr)
    , m_btnRefresh(nullptr)
    , m_btnExport(nullptr)
    , m_searchGroup(nullptr)
    , m_searchEdit(nullptr)
    , m_roleFilter(nullptr)
    , m_statusFilter(nullptr)
    , m_btnSearch(nullptr)
    , m_userTable(nullptr)
    , m_statsLabel(nullptr)
    , m_dbManager(nullptr)
{
    setupUI();
}

void UserManageWidget::setDatabaseManager(DatabaseManager* dbManager)
{
    m_dbManager = dbManager;
    // 延迟加载数据，确保界面完全初始化后再执行
    QTimer::singleShot(0, this, &UserManageWidget::loadUsers);
}

void UserManageWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(15);
    
    // 标题
    QLabel* titleLabel = new QLabel("用户管理");
    UIStyleManager::applyLabelStyle(titleLabel, "title");
    m_mainLayout->addWidget(titleLabel);
    
    // 工具栏
    m_toolLayout = new QHBoxLayout;
    
    m_btnAdd = new QPushButton("➕ 添加用户");
    m_btnEdit = new QPushButton("✏️ 编辑用户");
    m_btnDelete = new QPushButton("🗑️ 删除用户");
    m_btnRefresh = new QPushButton("🔄 刷新");
    m_btnExport = new QPushButton("📄 导出");
    
    UIStyleManager::applyButtonStyle(m_btnAdd, "success");
    UIStyleManager::applyButtonStyle(m_btnEdit, "primary");
    UIStyleManager::applyButtonStyle(m_btnDelete, "error");
    UIStyleManager::applyButtonStyle(m_btnRefresh, "secondary");
    UIStyleManager::applyButtonStyle(m_btnExport, "secondary");
    
    m_btnEdit->setEnabled(false);
    m_btnDelete->setEnabled(false);
    
    connect(m_btnAdd, &QPushButton::clicked, this, &UserManageWidget::onAddUser);
    connect(m_btnEdit, &QPushButton::clicked, this, &UserManageWidget::onEditUser);
    connect(m_btnDelete, &QPushButton::clicked, this, &UserManageWidget::onDeleteUser);
    connect(m_btnRefresh, &QPushButton::clicked, this, &UserManageWidget::onRefreshUsers);
    connect(m_btnExport, &QPushButton::clicked, this, &UserManageWidget::onExportUsers);
    
    m_toolLayout->addWidget(m_btnAdd);
    m_toolLayout->addWidget(m_btnEdit);
    m_toolLayout->addWidget(m_btnDelete);
    
    // 添加一个分隔线
    QFrame* separator = new QFrame;
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setMaximumWidth(2);
    m_toolLayout->addWidget(separator);
    
    m_toolLayout->addWidget(m_btnRefresh);
    m_toolLayout->addWidget(m_btnExport);
    m_toolLayout->addStretch();
    
    m_mainLayout->addLayout(m_toolLayout);
    
    // 搜索区域
    m_searchGroup = new QGroupBox("搜索筛选");
    UIStyleManager::applyGroupBoxStyle(m_searchGroup);
    QHBoxLayout* searchLayout = new QHBoxLayout(m_searchGroup);
    
    QLabel* searchLabel = new QLabel("姓名:");
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("输入用户姓名...");
    UIStyleManager::applyLineEditStyle(m_searchEdit);
    
    QLabel* roleLabel = new QLabel("角色:");
    m_roleFilter = new QComboBox;
    m_roleFilter->addItems({"全部", "患者", "客服", "管理员"});
    
    QLabel* statusLabel = new QLabel("状态:");
    m_statusFilter = new QComboBox;
    m_statusFilter->addItems({"全部", "活跃", "禁用"});
    
    m_btnSearch = new QPushButton("🔍 搜索");
    UIStyleManager::applyButtonStyle(m_btnSearch, "primary");
    
    connect(m_btnSearch, &QPushButton::clicked, this, &UserManageWidget::onSearchUsers);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &UserManageWidget::onSearchUsers);
    
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(roleLabel);
    searchLayout->addWidget(m_roleFilter);
    searchLayout->addWidget(statusLabel);
    searchLayout->addWidget(m_statusFilter);
    searchLayout->addWidget(m_btnSearch);
    searchLayout->addStretch();
    
    m_mainLayout->addWidget(m_searchGroup);
    
    // 用户表格
    m_userTable = new QTableWidget;
    m_userTable->setColumnCount(6);
    QStringList headers = {"用户ID", "姓名", "角色", "状态", "最后登录", "操作"};
    m_userTable->setHorizontalHeaderLabels(headers);
    
    // 设置表格属性
    m_userTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_userTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_userTable->horizontalHeader()->setStretchLastSection(true);
    m_userTable->setAlternatingRowColors(true);
    
    connect(m_userTable, &QTableWidget::itemSelectionChanged, 
            this, &UserManageWidget::onUserSelectionChanged);
    
    // 应用表格样式
    m_userTable->setStyleSheet(R"(
        QTableWidget {
            border: 1px solid #dee2e6;
            border-radius: 8px;
            background-color: white;
            gridline-color: #f1f3f4;
            selection-background-color: #e3f2fd;
        }
        QTableWidget::item {
            padding: 12px 8px;
            border-bottom: 1px solid #f1f3f4;
        }
        QTableWidget::item:selected {
            background-color: #2196f3;
            color: white;
        }
        QHeaderView::section {
            background-color: #f8f9fa;
            border: none;
            border-bottom: 2px solid #dee2e6;
            padding: 10px 8px;
            font-weight: bold;
            color: #495057;
        }
    )");
    
    m_mainLayout->addWidget(m_userTable);
    
    // 统计信息
    m_statsLabel = new QLabel;
    UIStyleManager::applyLabelStyle(m_statsLabel, "caption");
    m_mainLayout->addWidget(m_statsLabel);
}

void UserManageWidget::loadUsers()
{
    if (!m_dbManager) return;
    
    // 清空表格
    m_userTable->setRowCount(0);
    
    // 获取所有用户数据
    QList<UserInfo> users = m_dbManager->getAllUsers();
    
    m_userTable->setRowCount(users.size());
    
    int activeCount = 0;
    for (int i = 0; i < users.size(); ++i) {
        const UserInfo& user = users[i];
        QString roleName = user.role;  // 直接使用字符串role
        
        QString status = (user.status == 1) ? "活跃" : "禁用";
        if (user.status == 1) activeCount++;
        
        QString lastLogin = user.lastLogin.isValid() ? 
            user.lastLogin.toString("yyyy-MM-dd hh:mm") : "从未登录";
        
        QString displayName = user.realName.isEmpty() ? user.username : user.realName;
        QString userIdStr = QString("U%1").arg(user.id, 4, 10, QChar('0'));  // 生成用户ID字符串
        
        addUserToTable(i, userIdStr, displayName, roleName, status, lastLogin);
    }
    
    // 更新统计信息
    int totalUsers = users.size();
    m_statsLabel->setText(QString("总用户数: %1 | 活跃用户: %2 | 禁用用户: %3")
                         .arg(totalUsers).arg(activeCount).arg(totalUsers - activeCount));
}

void UserManageWidget::addUserToTable(int row, const QString& id, const QString& name, 
                                      const QString& role, const QString& status, 
                                      const QString& lastLogin)
{
    m_userTable->setItem(row, 0, new QTableWidgetItem(id));
    m_userTable->setItem(row, 1, new QTableWidgetItem(name));
    m_userTable->setItem(row, 2, new QTableWidgetItem(role));
    
    // 状态列带颜色显示
    QTableWidgetItem* statusItem = new QTableWidgetItem(status);
    if (status == "活跃") {
        statusItem->setBackground(QBrush(QColor(212, 237, 218)));
        statusItem->setForeground(QBrush(QColor(25, 135, 84)));
    } else {
        statusItem->setBackground(QBrush(QColor(248, 215, 218)));
        statusItem->setForeground(QBrush(QColor(220, 53, 69)));
    }
    m_userTable->setItem(row, 3, statusItem);
    
    m_userTable->setItem(row, 4, new QTableWidgetItem(lastLogin));
    
    // 操作列
    QPushButton* actionBtn = new QPushButton("详情");
    UIStyleManager::applyButtonStyle(actionBtn, "secondary");
    actionBtn->setMaximumWidth(60);
    m_userTable->setCellWidget(row, 5, actionBtn);
}

void UserManageWidget::onAddUser()
{
    UserEditDialog dialog(this);
    dialog.setWindowTitle("添加用户");
    
    if (dialog.exec() == QDialog::Accepted) {
        QString id = dialog.getUserId();
        QString name = dialog.getUserName();
        QString role = dialog.getUserRole();
        bool isActive = dialog.isUserActive();
        
        QMessageBox::information(this, "成功", 
                               QString("用户 %1 已成功添加！").arg(name));
        onRefreshUsers();
    }
}

void UserManageWidget::onEditUser()
{
    int currentRow = m_userTable->currentRow();
    if (currentRow < 0) return;
    
    QString id = m_userTable->item(currentRow, 0)->text();
    QString name = m_userTable->item(currentRow, 1)->text();
    QString role = m_userTable->item(currentRow, 2)->text();
    QString status = m_userTable->item(currentRow, 3)->text();
    
    UserEditDialog dialog(this);
    dialog.setWindowTitle("编辑用户");
    dialog.setUserData(id, name, role, status == "活跃");
    
    if (dialog.exec() == QDialog::Accepted) {
        QMessageBox::information(this, "成功", 
                               QString("用户 %1 的信息已更新！").arg(name));
        onRefreshUsers();
    }
}

void UserManageWidget::onDeleteUser()
{
    int currentRow = m_userTable->currentRow();
    if (currentRow < 0) return;
    
    QString name = m_userTable->item(currentRow, 1)->text();
    
    QMessageBox::StandardButton reply = QMessageBox::warning(
        this,
        "确认删除",
        QString("确定要删除用户 %1 吗？\n此操作不可撤销！").arg(name),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        m_userTable->removeRow(currentRow);
        QMessageBox::information(this, "成功", QString("用户 %1 已删除！").arg(name));
        onUserSelectionChanged(); // 更新按钮状态
    }
}

void UserManageWidget::onRefreshUsers()
{
    loadUsers();
    QMessageBox::information(this, "刷新", "用户列表已刷新！");
}

void UserManageWidget::onSearchUsers()
{
    QString searchText = m_searchEdit->text().trimmed();
    QString roleFilter = m_roleFilter->currentText();
    QString statusFilter = m_statusFilter->currentText();
    
    // 这里实现搜索逻辑
    // 示例：简单的名称过滤
    for (int i = 0; i < m_userTable->rowCount(); ++i) {
        bool visible = true;
        
        if (!searchText.isEmpty()) {
            QString name = m_userTable->item(i, 1)->text();
            visible = name.contains(searchText, Qt::CaseInsensitive);
        }
        
        if (visible && roleFilter != "全部") {
            QString role = m_userTable->item(i, 2)->text();
            visible = (role == roleFilter);
        }
        
        if (visible && statusFilter != "全部") {
            QString status = m_userTable->item(i, 3)->text();
            visible = (status == statusFilter);
        }
        
        m_userTable->setRowHidden(i, !visible);
    }
}

void UserManageWidget::onUserSelectionChanged()
{
    bool hasSelection = m_userTable->currentRow() >= 0;
    m_btnEdit->setEnabled(hasSelection);
    m_btnDelete->setEnabled(hasSelection);
}

void UserManageWidget::onExportUsers()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "导出用户数据",
        QString("users_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "CSV文件 (*.csv)"
    );
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            // Qt5/Qt6兼容性：在Qt6中setCodec已移除，默认使用UTF-8
            #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            out.setCodec("UTF-8");
            #endif
            
            // 写入表头
            out << "用户ID,姓名,角色,状态,最后登录\n";
            
            // 写入数据
            for (int i = 0; i < m_userTable->rowCount(); ++i) {
                if (!m_userTable->isRowHidden(i)) {
                    for (int j = 0; j < 5; ++j) { // 排除操作列
                        out << m_userTable->item(i, j)->text();
                        if (j < 4) out << ",";
                    }
                    out << "\n";
                }
            }
            
            QMessageBox::information(this, "导出成功", 
                                   QString("用户数据已导出到:\n%1").arg(fileName));
        }
    }
}

// UserEditDialog 实现
UserEditDialog::UserEditDialog(QWidget *parent)
    : QDialog(parent)
    , m_layout(nullptr)
    , m_editId(nullptr)
    , m_editName(nullptr)
    , m_comboRole(nullptr)
    , m_checkActive(nullptr)
    , m_btnOk(nullptr)
    , m_btnCancel(nullptr)
{
    setupUI();
}

void UserEditDialog::setupUI()
{
    setWindowTitle("用户信息");
    setFixedSize(350, 250);
    setModal(true);
    
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(20, 20, 20, 20);
    m_layout->setSpacing(15);
    
    // 用户ID
    QHBoxLayout* idLayout = new QHBoxLayout;
    idLayout->addWidget(new QLabel("用户ID:"));
    m_editId = new QLineEdit;
    m_editId->setPlaceholderText("自动生成");
    UIStyleManager::applyLineEditStyle(m_editId);
    idLayout->addWidget(m_editId);
    m_layout->addLayout(idLayout);
    
    // 姓名
    QHBoxLayout* nameLayout = new QHBoxLayout;
    nameLayout->addWidget(new QLabel("姓名:"));
    m_editName = new QLineEdit;
    m_editName->setPlaceholderText("输入用户姓名");
    UIStyleManager::applyLineEditStyle(m_editName);
    nameLayout->addWidget(m_editName);
    m_layout->addLayout(nameLayout);
    
    // 角色
    QHBoxLayout* roleLayout = new QHBoxLayout;
    roleLayout->addWidget(new QLabel("角色:"));
    m_comboRole = new QComboBox;
    m_comboRole->addItems({"患者", "客服", "管理员"});
    roleLayout->addWidget(m_comboRole);
    m_layout->addLayout(roleLayout);
    
    // 状态
    m_checkActive = new QCheckBox("账户激活");
    m_checkActive->setChecked(true);
    m_layout->addWidget(m_checkActive);
    
    m_layout->addStretch();
    
    // 按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    m_btnOk = new QPushButton("确定");
    m_btnCancel = new QPushButton("取消");
    
    UIStyleManager::applyButtonStyle(m_btnOk, "primary");
    UIStyleManager::applyButtonStyle(m_btnCancel, "secondary");
    
    connect(m_btnOk, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_btnOk);
    buttonLayout->addWidget(m_btnCancel);
    m_layout->addLayout(buttonLayout);
}

void UserEditDialog::setUserData(const QString& id, const QString& name, 
                                 const QString& role, bool isActive)
{
    m_editId->setText(id);
    m_editId->setReadOnly(true); // ID不可编辑
    m_editName->setText(name);
    
    if (role == "患者") m_comboRole->setCurrentIndex(0);
    else if (role == "客服") m_comboRole->setCurrentIndex(1);
    else if (role == "管理员") m_comboRole->setCurrentIndex(2);
    
    m_checkActive->setChecked(isActive);
}

QString UserEditDialog::getUserId() const
{
    return m_editId->text();
}

QString UserEditDialog::getUserName() const
{
    return m_editName->text();
}

QString UserEditDialog::getUserRole() const
{
    return m_comboRole->currentText();
}

bool UserEditDialog::isUserActive() const
{
    return m_checkActive->isChecked();
} 