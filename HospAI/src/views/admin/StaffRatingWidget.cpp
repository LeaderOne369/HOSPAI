#include "StaffRatingWidget.h"
#include "../common/UIStyleManager.h"
#include <QTimer>

StaffRatingWidget::StaffRatingWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_dbManager(nullptr)
{
    setupUI();
}

void StaffRatingWidget::setDatabaseManager(DatabaseManager* dbManager)
{
    m_dbManager = dbManager;
    // 延迟加载数据，确保界面完全初始化后再执行
    QTimer::singleShot(0, this, &StaffRatingWidget::loadRatings);
}

void StaffRatingWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    m_mainLayout->setSpacing(15);
    
    // 标题
    QLabel* titleLabel = new QLabel("客服满意度评价管理", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2C3E50; margin-bottom: 10px;");
    m_mainLayout->addWidget(titleLabel);
    
    // 创建主要内容区域
    m_topLayout = new QHBoxLayout;
    m_topLayout->setSpacing(15);
    
    // 左侧统计面板
    setupStatsPanel();
    
    // 右侧评价列表
    m_rightPanel = new QWidget(this);
    m_rightLayout = new QVBoxLayout(m_rightPanel);
    m_rightLayout->setContentsMargins(0, 0, 0, 0);
    m_rightLayout->setSpacing(10);
    
    // 顶部工具栏
    m_toolLayout = new QHBoxLayout;
    m_toolLayout->setSpacing(10);
    
    m_btnRefresh = new QPushButton("🔄 刷新", this);
    m_btnExport = new QPushButton("📤 导出", this);
    m_btnViewDetails = new QPushButton("📋 查看详情", this);
    m_btnStaffStats = new QPushButton("📊 客服统计", this);
    
    UIStyleManager::applyButtonStyle(m_btnRefresh, "secondary");
    UIStyleManager::applyButtonStyle(m_btnExport, "secondary");
    UIStyleManager::applyButtonStyle(m_btnViewDetails, "secondary");
    UIStyleManager::applyButtonStyle(m_btnStaffStats, "primary");
    
    m_toolLayout->addWidget(m_btnRefresh);
    m_toolLayout->addWidget(m_btnExport);
    m_toolLayout->addWidget(m_btnViewDetails);
    m_toolLayout->addWidget(m_btnStaffStats);
    m_toolLayout->addStretch();
    
    m_rightLayout->addLayout(m_toolLayout);
    
    // 搜索区域
    m_searchGroup = new QGroupBox("搜索条件", this);
    UIStyleManager::applyGroupBoxStyle(m_searchGroup);
    
    QHBoxLayout* searchLayout = new QHBoxLayout(m_searchGroup);
    searchLayout->setSpacing(10);
    
    searchLayout->addWidget(new QLabel("客服:", this));
    m_staffFilter = new QComboBox(this);
    m_staffFilter->addItem("全部客服");
    // UIStyleManager::applyComboBoxStyle(m_staffFilter); // 暂时注释掉
    searchLayout->addWidget(m_staffFilter);
    
    searchLayout->addWidget(new QLabel("评分:", this));
    m_ratingFilter = new QComboBox(this);
    m_ratingFilter->addItems({"全部", "5星", "4星", "3星", "2星", "1星"});
    // UIStyleManager::applyComboBoxStyle(m_ratingFilter); // 暂时注释掉
    searchLayout->addWidget(m_ratingFilter);
    
    searchLayout->addWidget(new QLabel("时间:", this));
    m_dateFromEdit = new QDateEdit(QDate::currentDate().addDays(-30), this);
    m_dateToEdit = new QDateEdit(QDate::currentDate(), this);
    m_dateFromEdit->setCalendarPopup(true);
    m_dateToEdit->setCalendarPopup(true);
    searchLayout->addWidget(m_dateFromEdit);
    searchLayout->addWidget(new QLabel("至", this));
    searchLayout->addWidget(m_dateToEdit);
    
    m_btnSearch = new QPushButton("🔍 搜索", this);
    UIStyleManager::applyButtonStyle(m_btnSearch, "primary");
    searchLayout->addWidget(m_btnSearch);
    
    m_rightLayout->addWidget(m_searchGroup);
    
    // 评价列表表格
    m_ratingTable = new QTableWidget(this);
    m_ratingTable->setColumnCount(6);
    QStringList headers = {"患者", "客服", "评分", "评价内容", "评价时间", "会话时长"};
    m_ratingTable->setHorizontalHeaderLabels(headers);
    
    // 设置表格属性
    m_ratingTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ratingTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_ratingTable->setAlternatingRowColors(true);
    m_ratingTable->setSortingEnabled(true);
    
    // UIStyleManager::applyTableStyle(m_ratingTable); // 暂时注释掉
    m_rightLayout->addWidget(m_ratingTable);
    
    // 统计信息
    m_statsLabel = new QLabel("共 0 条评价", this);
    m_statsLabel->setStyleSheet("color: #7F8C8D; font-size: 12px;");
    m_rightLayout->addWidget(m_statsLabel);
    
    // 添加到主布局
    m_topLayout->addWidget(m_leftPanel, 1);
    m_topLayout->addWidget(m_rightPanel, 2);
    m_mainLayout->addLayout(m_topLayout);
    
    // 连接信号槽
    connect(m_btnRefresh, &QPushButton::clicked, this, &StaffRatingWidget::onRefreshRatings);
    connect(m_btnExport, &QPushButton::clicked, [this]() { QMessageBox::information(this, "提示", "导出评价功能待实现"); });
    connect(m_btnViewDetails, &QPushButton::clicked, [this]() { QMessageBox::information(this, "提示", "查看评价详情功能待实现"); });
    connect(m_btnStaffStats, &QPushButton::clicked, [this]() { QMessageBox::information(this, "提示", "客服统计功能待实现"); });
    connect(m_btnSearch, &QPushButton::clicked, this, &StaffRatingWidget::onSearchRatings);
}

void StaffRatingWidget::setupStatsPanel()
{
    m_leftPanel = new QWidget(this);
    m_leftLayout = new QVBoxLayout(m_leftPanel);
    m_leftLayout->setContentsMargins(0, 0, 0, 0);
    m_leftLayout->setSpacing(15);
    
    // 统计信息组
    m_statsGroup = new QGroupBox("评价统计", this);
    UIStyleManager::applyGroupBoxStyle(m_statsGroup);
    
    QVBoxLayout* statsLayout = new QVBoxLayout(m_statsGroup);
    statsLayout->setSpacing(10);
    
    // 基础统计
    m_totalRatingsLabel = new QLabel("总评价数: 0", this);
    m_avgRatingLabel = new QLabel("平均评分: 0.0", this);
    m_bestStaffLabel = new QLabel("最佳客服: --", this);
    m_worstStaffLabel = new QLabel("待提升客服: --", this);
    
    m_totalRatingsLabel->setStyleSheet("font-size: 14px; margin: 5px;");
    m_avgRatingLabel->setStyleSheet("font-size: 14px; margin: 5px; color: #E67E22;");
    m_bestStaffLabel->setStyleSheet("font-size: 14px; margin: 5px; color: #27AE60;");
    m_worstStaffLabel->setStyleSheet("font-size: 14px; margin: 5px; color: #E74C3C;");
    
    statsLayout->addWidget(m_totalRatingsLabel);
    statsLayout->addWidget(m_avgRatingLabel);
    statsLayout->addWidget(m_bestStaffLabel);
    statsLayout->addWidget(m_worstStaffLabel);
    
    // 满意度分布（简化版）
    statsLayout->addWidget(new QLabel("满意度分布:", this));
    
    // 5星满意度
    QHBoxLayout* star5Layout = new QHBoxLayout;
    star5Layout->addWidget(new QLabel("⭐⭐⭐⭐⭐", this));
    m_satisfaction5Star = new QProgressBar(this);
    m_satisfaction5Star->setRange(0, 100);
    m_satisfaction5Star->setValue(0);
    m_satisfaction5Star->setTextVisible(true);
    star5Layout->addWidget(m_satisfaction5Star);
    statsLayout->addLayout(star5Layout);
    
    // 4星满意度
    QHBoxLayout* star4Layout = new QHBoxLayout;
    star4Layout->addWidget(new QLabel("⭐⭐⭐⭐", this));
    m_satisfaction4Star = new QProgressBar(this);
    m_satisfaction4Star->setRange(0, 100);
    m_satisfaction4Star->setValue(0);
    m_satisfaction4Star->setTextVisible(true);
    star4Layout->addWidget(m_satisfaction4Star);
    statsLayout->addLayout(star4Layout);
    
    // 3星满意度
    QHBoxLayout* star3Layout = new QHBoxLayout;
    star3Layout->addWidget(new QLabel("⭐⭐⭐", this));
    m_satisfaction3Star = new QProgressBar(this);
    m_satisfaction3Star->setRange(0, 100);
    m_satisfaction3Star->setValue(0);
    m_satisfaction3Star->setTextVisible(true);
    star3Layout->addWidget(m_satisfaction3Star);
    statsLayout->addLayout(star3Layout);
    
    // 2星满意度
    QHBoxLayout* star2Layout = new QHBoxLayout;
    star2Layout->addWidget(new QLabel("⭐⭐", this));
    m_satisfaction2Star = new QProgressBar(this);
    m_satisfaction2Star->setRange(0, 100);
    m_satisfaction2Star->setValue(0);
    m_satisfaction2Star->setTextVisible(true);
    star2Layout->addWidget(m_satisfaction2Star);
    statsLayout->addLayout(star2Layout);
    
    // 1星满意度
    QHBoxLayout* star1Layout = new QHBoxLayout;
    star1Layout->addWidget(new QLabel("⭐", this));
    m_satisfaction1Star = new QProgressBar(this);
    m_satisfaction1Star->setRange(0, 100);
    m_satisfaction1Star->setValue(0);
    m_satisfaction1Star->setTextVisible(true);
    star1Layout->addWidget(m_satisfaction1Star);
    statsLayout->addLayout(star1Layout);
    
    m_leftLayout->addWidget(m_statsGroup);
    m_leftLayout->addStretch();
}

void StaffRatingWidget::loadRatings()
{
    if (!m_dbManager) return;
    
    // 清空表格
    m_ratingTable->setRowCount(0);
    
    // 目前数据库方法暂未实现，先使用模拟数据
    // TODO: 实现 m_dbManager->getAllSessionRatings()
    QList<SessionRating> ratings;
    
    // 模拟一些评价数据
    SessionRating rating1;
    rating1.id = 1;
    rating1.sessionId = 1;
    rating1.patientId = "P001";
    rating1.staffId = "S001";
    rating1.rating = 5;
    rating1.comment = "客服态度很好，回复很及时";
    rating1.createdAt = QDateTime::currentDateTime().addDays(-1);
    rating1.ratingTime = rating1.createdAt;
    ratings.append(rating1);
    
    SessionRating rating2;
    rating2.id = 2;
    rating2.sessionId = 2;
    rating2.patientId = "P002";
    rating2.staffId = "S001";
    rating2.rating = 4;
    rating2.comment = "服务不错，但等待时间有点长";
    rating2.createdAt = QDateTime::currentDateTime().addDays(-2);
    rating2.ratingTime = rating2.createdAt;
    ratings.append(rating2);
    
    // 获取用户信息映射
    QList<UserInfo> users = m_dbManager->getAllUsers();
    QMap<QString, UserInfo> userMap;
    for (const UserInfo& user : users) {
        QString userIdStr = QString("U%1").arg(user.id, 4, 10, QChar('0'));
        // 根据角色生成不同前缀的ID
        if (user.role == "患者") userIdStr = QString("P%1").arg(user.id, 3, 10, QChar('0'));
        else if (user.role == "客服") userIdStr = QString("S%1").arg(user.id, 3, 10, QChar('0'));
        else if (user.role == "管理员") userIdStr = QString("A%1").arg(user.id, 3, 10, QChar('0'));
        
        userMap[userIdStr] = user;
    }
    
    m_ratingTable->setRowCount(ratings.size());
    
    for (int i = 0; i < ratings.size(); ++i) {
        const SessionRating& rating = ratings[i];
        
        // 获取患者和客服信息
        QString patientName = userMap.contains(rating.patientId) ? 
            (userMap[rating.patientId].realName.isEmpty() ? userMap[rating.patientId].username : userMap[rating.patientId].realName) : rating.patientId;
        QString staffName = userMap.contains(rating.staffId) ? 
            (userMap[rating.staffId].realName.isEmpty() ? userMap[rating.staffId].username : userMap[rating.staffId].realName) : rating.staffId;
        
        addRatingToTable(i, rating, patientName, staffName);
    }
    
    updateStatsPanel(ratings);
    
    // 更新统计信息
    m_statsLabel->setText(QString("共 %1 条评价").arg(ratings.size()));
}

void StaffRatingWidget::addRatingToTable(int row, const SessionRating& rating, const QString& patientName, const QString& staffName)
{
    // 患者
    QTableWidgetItem* patientItem = new QTableWidgetItem(patientName);
    m_ratingTable->setItem(row, 0, patientItem);
    
    // 客服
    QTableWidgetItem* staffItem = new QTableWidgetItem(staffName);
    m_ratingTable->setItem(row, 1, staffItem);
    
    // 评分
    QString ratingText = QString("⭐").repeated(rating.rating) + QString(" (%1)").arg(rating.rating);
    QTableWidgetItem* ratingItem = new QTableWidgetItem(ratingText);
    m_ratingTable->setItem(row, 2, ratingItem);
    
    // 评价内容
    QString comment = rating.comment.length() > 50 ? 
        rating.comment.left(47) + "..." : rating.comment;
    QTableWidgetItem* commentItem = new QTableWidgetItem(comment);
    m_ratingTable->setItem(row, 3, commentItem);
    
    // 评价时间
    QTableWidgetItem* timeItem = new QTableWidgetItem(rating.createdAt.toString("yyyy-MM-dd hh:mm"));
    m_ratingTable->setItem(row, 4, timeItem);
    
    // 会话时长（如果有会话信息）
    // 这里需要查询会话信息获取时长，暂时显示默认值
    QTableWidgetItem* durationItem = new QTableWidgetItem("--");
    m_ratingTable->setItem(row, 5, durationItem);
}

void StaffRatingWidget::updateStatsPanel(const QList<SessionRating>& ratings)
{
    if (ratings.isEmpty()) {
        m_totalRatingsLabel->setText("总评价数: 0");
        m_avgRatingLabel->setText("平均评分: 0.0");
        m_bestStaffLabel->setText("最佳客服: --");
        m_worstStaffLabel->setText("待提升客服: --");
        
        m_satisfaction5Star->setValue(0);
        m_satisfaction4Star->setValue(0);
        m_satisfaction3Star->setValue(0);
        m_satisfaction2Star->setValue(0);
        m_satisfaction1Star->setValue(0);
        return;
    }
    
    // 计算总评价数和平均评分
    int totalRatings = ratings.size();
    double totalScore = 0;
    QMap<int, int> ratingCounts;
    QMap<QString, QList<int>> staffRatings;
    
    for (const SessionRating& rating : ratings) {
        totalScore += rating.rating;
        ratingCounts[rating.rating]++;
        staffRatings[rating.staffId].append(rating.rating);
    }
    
    double avgRating = totalScore / totalRatings;
    
    // 更新基础统计
    m_totalRatingsLabel->setText(QString("总评价数: %1").arg(totalRatings));
    m_avgRatingLabel->setText(QString("平均评分: %1").arg(avgRating, 0, 'f', 1));
    
    // 计算最佳和待提升客服
    QString bestStaff = "--", worstStaff = "--";
    double bestScore = 0, worstScore = 6;
    
    for (auto it = staffRatings.begin(); it != staffRatings.end(); ++it) {
        const QList<int>& ratings = it.value();
        double staffAvg = 0;
        for (int rating : ratings) {
            staffAvg += rating;
        }
        staffAvg /= ratings.size();
        
        if (staffAvg > bestScore) {
            bestScore = staffAvg;
            bestStaff = QString("%1 (%2分)").arg(it.key()).arg(staffAvg, 0, 'f', 1);
        }
        if (staffAvg < worstScore) {
            worstScore = staffAvg;
            worstStaff = QString("%1 (%2分)").arg(it.key()).arg(staffAvg, 0, 'f', 1);
        }
    }
    
    m_bestStaffLabel->setText(QString("最佳客服: %1").arg(bestStaff));
    m_worstStaffLabel->setText(QString("待提升客服: %1").arg(worstStaff));
    
    // 更新满意度分布
    int count5 = ratingCounts.value(5, 0);
    int count4 = ratingCounts.value(4, 0);
    int count3 = ratingCounts.value(3, 0);
    int count2 = ratingCounts.value(2, 0);
    int count1 = ratingCounts.value(1, 0);
    
    m_satisfaction5Star->setValue(totalRatings > 0 ? count5 * 100 / totalRatings : 0);
    m_satisfaction4Star->setValue(totalRatings > 0 ? count4 * 100 / totalRatings : 0);
    m_satisfaction3Star->setValue(totalRatings > 0 ? count3 * 100 / totalRatings : 0);
    m_satisfaction2Star->setValue(totalRatings > 0 ? count2 * 100 / totalRatings : 0);
    m_satisfaction1Star->setValue(totalRatings > 0 ? count1 * 100 / totalRatings : 0);
}

void StaffRatingWidget::onRefreshRatings()
{
    loadRatings();
}

void StaffRatingWidget::onSearchRatings()
{
    QMessageBox::information(this, "提示", "搜索功能开发中...");
}

// 其他槽函数的简化实现
void StaffRatingWidget::onRatingSelectionChanged() { /* 暂时留空 */ }
void StaffRatingWidget::onExportRatings() { QMessageBox::information(this, "提示", "功能开发中..."); }
void StaffRatingWidget::onViewRatingDetails() { QMessageBox::information(this, "提示", "功能开发中..."); }
void StaffRatingWidget::onShowStaffStats() { QMessageBox::information(this, "提示", "功能开发中..."); }
void StaffRatingWidget::onDateRangeChanged() { /* 暂时留空 */ }
void StaffRatingWidget::setupRatingChart() { /* 暂时留空 */ }
void StaffRatingWidget::updateRatingChart() { /* 暂时留空 */ }
void StaffRatingWidget::showRatingDetailsDialog(const SessionRating& rating) { Q_UNUSED(rating); QMessageBox::information(this, "提示", "功能开发中..."); }

// ========== 对话框类的简化实现 ==========

RatingDetailsDialog::RatingDetailsDialog(const SessionRating& rating, const UserInfo& staff, const UserInfo& patient, QWidget *parent)
    : QDialog(parent)
    , m_rating(rating)
    , m_staff(staff)
    , m_patient(patient)
{
    setWindowTitle("评价详情（开发中）");
    resize(300, 200);
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("评价详情功能正在开发中...", this));
    
    QPushButton* closeButton = new QPushButton("关闭", this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(closeButton);
}

void RatingDetailsDialog::setupUI() { /* 简化实现 */ }

StaffStatsDialog::StaffStatsDialog(DatabaseManager* dbManager, QWidget *parent)
    : QDialog(parent)
    , m_dbManager(dbManager)
{
    setWindowTitle("客服统计（开发中）");
    resize(400, 300);
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("客服统计功能正在开发中...", this));
    
    QPushButton* closeButton = new QPushButton("关闭", this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(closeButton);
}

void StaffStatsDialog::setupUI() { /* 简化实现 */ }
void StaffStatsDialog::onRefreshStats() { /* 简化实现 */ }
void StaffStatsDialog::onStaffSelectionChanged() { /* 简化实现 */ }
void StaffStatsDialog::loadStaffStats() { /* 简化实现 */ }
void StaffStatsDialog::addStaffToTable(int row, const UserInfo& staff, double avgRating, int totalRatings) { Q_UNUSED(row); Q_UNUSED(staff); Q_UNUSED(avgRating); Q_UNUSED(totalRatings); }

// MOC generated automatically
