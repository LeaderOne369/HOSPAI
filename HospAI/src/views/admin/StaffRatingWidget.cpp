#include "StaffRatingWidget.h"
#include "../common/UIStyleManager.h"
#include <QTimer>
#include <QFileDialog>
#include <QTextStream>
#include <QDate>
#include <QFile>
#include <QGridLayout>
#include <QStringConverter>
#include <QBrush>

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
    connect(m_btnExport, &QPushButton::clicked, this, &StaffRatingWidget::onExportRatings);
    connect(m_btnViewDetails, &QPushButton::clicked, this, &StaffRatingWidget::onViewRatingDetails);
    connect(m_btnStaffStats, &QPushButton::clicked, this, &StaffRatingWidget::onShowStaffStats);
    connect(m_btnSearch, &QPushButton::clicked, this, &StaffRatingWidget::onSearchRatings);
    connect(m_ratingTable, &QTableWidget::itemSelectionChanged, this, &StaffRatingWidget::onRatingSelectionChanged);
    
    // 初始状态：禁用查看详情按钮
    m_btnViewDetails->setEnabled(false);
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
    
    // 从数据库加载真实的评价数据
    QList<SessionRating> ratings = m_dbManager->getAllSessionRatings();
    
    // 如果没有真实数据，生成一些示例数据用于演示
    if (ratings.isEmpty()) {
        // 模拟一些评价数据
        QList<UserInfo> users = m_dbManager->getAllUsers();
        QList<UserInfo> patients, staffs;
        
        for (const UserInfo& user : users) {
            if (user.role == "患者") patients.append(user);
            else if (user.role == "客服") staffs.append(user);
        }
        
        if (!patients.isEmpty() && !staffs.isEmpty()) {
            // 生成一些示例评价
            QStringList comments = {
                "客服态度很好，回复很及时，解决了我的问题",
                "服务不错，但等待时间有点长",
                "专业水平很高，解答很详细",
                "态度很好，很有耐心",
                "回复速度很快，满意",
                "服务质量一般，还有改进空间",
                "非常满意的服务体验",
                "客服很专业，推荐"
            };
            
            QList<int> ratingScores = {5, 4, 5, 4, 5, 3, 5, 4, 3, 5};
            
            for (int i = 0; i < qMin(10, patients.size() * staffs.size()); ++i) {
                SessionRating rating;
                rating.id = i + 1;
                rating.sessionId = i + 1;
                rating.patientId = QString::number(patients[i % patients.size()].id);
                rating.staffId = QString::number(staffs[i % staffs.size()].id);
                rating.rating = ratingScores[i % ratingScores.size()];
                rating.comment = comments[i % comments.size()];
                rating.createdAt = QDateTime::currentDateTime().addDays(-(i + 1));
                rating.ratingTime = rating.createdAt;
                ratings.append(rating);
            }
        }
    }
    
    // 获取用户信息映射
    QList<UserInfo> users = m_dbManager->getAllUsers();
    QMap<int, UserInfo> userMap;
    for (const UserInfo& user : users) {
        userMap[user.id] = user;
    }
    
    m_ratingTable->setRowCount(ratings.size());
    
    for (int i = 0; i < ratings.size(); ++i) {
        const SessionRating& rating = ratings[i];
        
        // 获取患者和客服信息
        int patientIdInt = rating.patientId.toInt();
        int staffIdInt = rating.staffId.toInt();
        
        QString patientName = userMap.contains(patientIdInt) ? 
            (userMap[patientIdInt].realName.isEmpty() ? userMap[patientIdInt].username : userMap[patientIdInt].realName) : 
            QString("患者%1").arg(rating.patientId);
        QString staffName = userMap.contains(staffIdInt) ? 
            (userMap[staffIdInt].realName.isEmpty() ? userMap[staffIdInt].username : userMap[staffIdInt].realName) : 
            QString("客服%1").arg(rating.staffId);
        
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
    QMap<int, QList<int>> staffRatings;
    
    for (const SessionRating& rating : ratings) {
        totalScore += rating.rating;
        ratingCounts[rating.rating]++;
        staffRatings[rating.staffId.toInt()].append(rating.rating);
    }
    
    double avgRating = totalScore / totalRatings;
    
    // 更新基础统计
    m_totalRatingsLabel->setText(QString("总评价数: %1").arg(totalRatings));
    m_avgRatingLabel->setText(QString("平均评分: %1").arg(avgRating, 0, 'f', 1));
    
    // 计算最佳和待提升客服
    QString bestStaff = "--", worstStaff = "--";
    double bestScore = 0, worstScore = 6;
    
    // 获取用户信息用于显示姓名
    QList<UserInfo> users = m_dbManager->getAllUsers();
    QMap<int, UserInfo> userMap;
    for (const UserInfo& user : users) {
        userMap[user.id] = user;
    }
    
    for (auto it = staffRatings.begin(); it != staffRatings.end(); ++it) {
        const QList<int>& ratings = it.value();
        double staffAvg = 0;
        for (int rating : ratings) {
            staffAvg += rating;
        }
        staffAvg /= ratings.size();
        
        QString staffName = userMap.contains(it.key()) ? 
            (userMap[it.key()].realName.isEmpty() ? userMap[it.key()].username : userMap[it.key()].realName) : 
            QString("客服%1").arg(it.key());
        
        if (staffAvg > bestScore) {
            bestScore = staffAvg;
            bestStaff = QString("%1 (%2分)").arg(staffName).arg(staffAvg, 0, 'f', 1);
        }
        if (staffAvg < worstScore) {
            worstScore = staffAvg;
            worstStaff = QString("%1 (%2分)").arg(staffName).arg(staffAvg, 0, 'f', 1);
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

void StaffRatingWidget::onRatingSelectionChanged() 
{
    int currentRow = m_ratingTable->currentRow();
    bool hasSelection = (currentRow >= 0 && currentRow < m_ratings.size());
    m_btnViewDetails->setEnabled(hasSelection);
}

void StaffRatingWidget::onExportRatings() 
{
    QString fileName = QFileDialog::getSaveFileName(this, 
                                                   "导出评价数据", 
                                                   QString("ratings_%1.csv").arg(QDate::currentDate().toString("yyyyMMdd")),
                                                   "CSV文件 (*.csv)");
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                         QTextStream stream(&file);
             stream.setEncoding(QStringConverter::Utf8);
            
            // 写入表头
            stream << "患者,客服,评分,评价内容,评价时间,会话ID\n";
            
            // 获取用户信息
            QList<UserInfo> users = m_dbManager->getAllUsers();
            QMap<int, UserInfo> userMap;
            for (const UserInfo& user : users) {
                userMap[user.id] = user;
            }
            
            // 写入数据
            for (const SessionRating& rating : m_ratings) {
                int patientIdInt = rating.patientId.toInt();
                int staffIdInt = rating.staffId.toInt();
                
                QString patientName = userMap.contains(patientIdInt) ? 
                    (userMap[patientIdInt].realName.isEmpty() ? userMap[patientIdInt].username : userMap[patientIdInt].realName) : 
                    QString("患者%1").arg(rating.patientId);
                QString staffName = userMap.contains(staffIdInt) ? 
                    (userMap[staffIdInt].realName.isEmpty() ? userMap[staffIdInt].username : userMap[staffIdInt].realName) : 
                    QString("客服%1").arg(rating.staffId);
                
                QString commentEscaped = rating.comment;
                commentEscaped.replace("\"", "\"\""); // CSV转义
                
                stream << QString("%1,%2,%3,\"%4\",%5,%6\n")
                         .arg(patientName)
                         .arg(staffName)
                         .arg(rating.rating)
                         .arg(commentEscaped)
                         .arg(rating.createdAt.toString("yyyy-MM-dd hh:mm:ss"))
                         .arg(rating.sessionId);
            }
            
            file.close();
            QMessageBox::information(this, "成功", QString("评价数据已导出到：%1").arg(fileName));
        } else {
            QMessageBox::critical(this, "错误", "导出文件失败！");
        }
    }
}

void StaffRatingWidget::onViewRatingDetails() 
{
    int currentRow = m_ratingTable->currentRow();
    if (currentRow < 0 || currentRow >= m_ratings.size()) {
        QMessageBox::warning(this, "提示", "请先选择要查看的评价！");
        return;
    }
    
    SessionRating rating = m_ratings[currentRow];
    
    // 获取患者和客服信息
    QList<UserInfo> users = m_dbManager->getAllUsers();
    UserInfo patient, staff;
    
    for (const UserInfo& user : users) {
        if (user.id == rating.patientId.toInt()) {
            patient = user;
        }
        if (user.id == rating.staffId.toInt()) {
            staff = user;
        }
    }
    
    RatingDetailsDialog dialog(rating, staff, patient, this);
    dialog.exec();
}

void StaffRatingWidget::onShowStaffStats() 
{
    StaffStatsDialog dialog(m_dbManager, this);
    dialog.exec();
}

void StaffRatingWidget::onDateRangeChanged() 
{
    // 当日期范围改变时，重新加载数据
    loadRatings();
}

void StaffRatingWidget::setupRatingChart() 
{
    // 预留的图表功能，暂时不实现
}

void StaffRatingWidget::updateRatingChart() 
{
    // 预留的图表功能，暂时不实现
}

void StaffRatingWidget::showRatingDetailsDialog(const SessionRating& rating) 
{
    // 获取患者和客服信息
    QList<UserInfo> users = m_dbManager->getAllUsers();
    UserInfo patient, staff;
    
    for (const UserInfo& user : users) {
        if (user.id == rating.patientId.toInt()) {
            patient = user;
        }
        if (user.id == rating.staffId.toInt()) {
            staff = user;
        }
    }
    
    RatingDetailsDialog dialog(rating, staff, patient, this);
    dialog.exec();
}

// ========== 对话框类的完整实现 ==========

RatingDetailsDialog::RatingDetailsDialog(const SessionRating& rating, const UserInfo& staff, const UserInfo& patient, QWidget *parent)
    : QDialog(parent)
    , m_rating(rating)
    , m_staff(staff)
    , m_patient(patient)
{
    setWindowTitle("评价详情");
    resize(500, 400);
    setupUI();
}

void RatingDetailsDialog::setupUI() 
{
    m_mainLayout = new QVBoxLayout(this);
    
    // 标题
    QLabel* titleLabel = new QLabel("会话评价详情", this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2C3E50; margin-bottom: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(titleLabel);
    
    // 基本信息区域
    QGroupBox* infoGroup = new QGroupBox("基本信息", this);
    QGridLayout* infoLayout = new QGridLayout(infoGroup);
    
    // 患者信息
    m_patientLabel = new QLabel(QString("患者: %1 (%2)")
                               .arg(m_patient.realName.isEmpty() ? m_patient.username : m_patient.realName)
                               .arg(m_patient.username), this);
    infoLayout->addWidget(m_patientLabel, 0, 0, 1, 2);
    
    // 客服信息
    m_staffLabel = new QLabel(QString("客服: %1 (%2)")
                             .arg(m_staff.realName.isEmpty() ? m_staff.username : m_staff.realName)
                             .arg(m_staff.username), this);
    infoLayout->addWidget(m_staffLabel, 1, 0, 1, 2);
    
    // 会话信息
    m_sessionLabel = new QLabel(QString("会话ID: %1").arg(m_rating.sessionId), this);
    infoLayout->addWidget(m_sessionLabel, 2, 0);
    
    // 评价时间
    m_timeLabel = new QLabel(QString("评价时间: %1").arg(m_rating.createdAt.toString("yyyy-MM-dd hh:mm:ss")), this);
    infoLayout->addWidget(m_timeLabel, 2, 1);
    
    m_mainLayout->addWidget(infoGroup);
    
    // 评分区域
    QGroupBox* ratingGroup = new QGroupBox("评分", this);
    QHBoxLayout* ratingLayout = new QHBoxLayout(ratingGroup);
    
    QString stars = QString("⭐").repeated(m_rating.rating);
    QString emptyStars = QString("☆").repeated(5 - m_rating.rating);
    m_ratingLabel = new QLabel(QString("%1%2 (%3/5分)").arg(stars).arg(emptyStars).arg(m_rating.rating), this);
    m_ratingLabel->setStyleSheet("font-size: 18px; color: #F39C12;");
    ratingLayout->addWidget(m_ratingLabel);
    ratingLayout->addStretch();
    
    m_mainLayout->addWidget(ratingGroup);
    
    // 评价内容区域
    QGroupBox* commentGroup = new QGroupBox("评价内容", this);
    QVBoxLayout* commentLayout = new QVBoxLayout(commentGroup);
    
    m_commentEdit = new QTextEdit(this);
    m_commentEdit->setPlainText(m_rating.comment);
    m_commentEdit->setReadOnly(true);
    m_commentEdit->setMaximumHeight(120);
    commentLayout->addWidget(m_commentEdit);
    
    m_mainLayout->addWidget(commentGroup);
    
    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    
    m_closeButton = new QPushButton("关闭", this);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(m_closeButton);
    
    m_mainLayout->addLayout(buttonLayout);
}

StaffStatsDialog::StaffStatsDialog(DatabaseManager* dbManager, QWidget *parent)
    : QDialog(parent)
    , m_dbManager(dbManager)
{
    setWindowTitle("客服评价统计");
    resize(700, 500);
    setupUI();
    loadStaffStats();
}

void StaffStatsDialog::setupUI() 
{
    m_mainLayout = new QVBoxLayout(this);
    
    // 标题
    QLabel* titleLabel = new QLabel("客服评价统计", this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2C3E50; margin-bottom: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(titleLabel);
    
    // 工具栏
    QHBoxLayout* toolLayout = new QHBoxLayout;
    
    m_refreshButton = new QPushButton("🔄 刷新", this);
    connect(m_refreshButton, &QPushButton::clicked, this, &StaffStatsDialog::onRefreshStats);
    toolLayout->addWidget(m_refreshButton);
    toolLayout->addStretch();
    
    m_mainLayout->addLayout(toolLayout);
    
    // 统计表格
    m_staffTable = new QTableWidget(this);
    m_staffTable->setColumnCount(6);
    QStringList headers = {"客服姓名", "用户名", "总评价数", "平均评分", "5星数", "1-2星数"};
    m_staffTable->setHorizontalHeaderLabels(headers);
    
    // 设置表格属性
    m_staffTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_staffTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_staffTable->setAlternatingRowColors(true);
    m_staffTable->setSortingEnabled(true);
    m_staffTable->horizontalHeader()->setStretchLastSection(true);
    
    connect(m_staffTable, &QTableWidget::itemSelectionChanged, this, &StaffStatsDialog::onStaffSelectionChanged);
    
    m_mainLayout->addWidget(m_staffTable);
    
    // 统计信息
    m_statsLabel = new QLabel("正在加载统计信息...", this);
    m_statsLabel->setStyleSheet("color: #7F8C8D; font-size: 12px;");
    m_mainLayout->addWidget(m_statsLabel);
    
    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    
    m_closeButton = new QPushButton("关闭", this);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(m_closeButton);
    
    m_mainLayout->addLayout(buttonLayout);
}

void StaffStatsDialog::onRefreshStats() 
{
    loadStaffStats();
}

void StaffStatsDialog::onStaffSelectionChanged() 
{
    // 可以在这里添加选中客服后的详细信息显示
}

void StaffStatsDialog::loadStaffStats() 
{
    if (!m_dbManager) return;
    
    // 获取所有客服
    QList<UserInfo> staffList = m_dbManager->getUsersByRole("客服");
    
    // 获取所有评价
    QList<SessionRating> allRatings = m_dbManager->getAllSessionRatings();
    
    // 统计每个客服的评价数据
    QMap<int, QList<int>> staffRatings; // staffId -> ratings list
    
    for (const SessionRating& rating : allRatings) {
        int staffId = rating.staffId.toInt();
        staffRatings[staffId].append(rating.rating);
    }
    
    // 更新表格
    m_staffTable->setRowCount(staffList.size());
    
    int totalStaff = 0;
    int totalRatings = 0;
    double totalScore = 0;
    
    for (int i = 0; i < staffList.size(); ++i) {
        const UserInfo& staff = staffList[i];
        const QList<int>& ratings = staffRatings[staff.id];
        
        if (!ratings.isEmpty()) {
            // 计算统计数据
            double avgRating = 0;
            int fiveStarCount = 0;
            int lowStarCount = 0; // 1-2星
            
            for (int rating : ratings) {
                avgRating += rating;
                if (rating == 5) fiveStarCount++;
                if (rating <= 2) lowStarCount++;
                totalScore += rating;
            }
            avgRating /= ratings.size();
            totalRatings += ratings.size();
            totalStaff++;
            
            addStaffToTable(i, staff, avgRating, ratings.size());
            
            // 设置额外数据
            m_staffTable->setItem(i, 4, new QTableWidgetItem(QString::number(fiveStarCount)));
            m_staffTable->setItem(i, 5, new QTableWidgetItem(QString::number(lowStarCount)));
            
            // 根据评分设置行颜色
            QColor rowColor;
            if (avgRating >= 4.5) {
                rowColor = QColor(212, 237, 218); // 绿色 - 优秀
            } else if (avgRating >= 3.5) {
                rowColor = QColor(255, 243, 205); // 黄色 - 良好
            } else {
                rowColor = QColor(248, 215, 218); // 红色 - 需要提升
            }
            
            for (int j = 0; j < m_staffTable->columnCount(); ++j) {
                if (m_staffTable->item(i, j)) {
                    m_staffTable->item(i, j)->setBackground(QBrush(rowColor));
                }
            }
        } else {
            // 没有评价的客服
            addStaffToTable(i, staff, 0, 0);
            m_staffTable->setItem(i, 4, new QTableWidgetItem("0"));
            m_staffTable->setItem(i, 5, new QTableWidgetItem("0"));
        }
    }
    
    // 更新统计信息
    double overallAvg = totalRatings > 0 ? totalScore / totalRatings : 0;
    m_statsLabel->setText(QString("共 %1 位客服，总计 %2 条评价，整体平均评分: %3")
                         .arg(staffList.size())
                         .arg(totalRatings)
                         .arg(overallAvg, 0, 'f', 1));
}

void StaffStatsDialog::addStaffToTable(int row, const UserInfo& staff, double avgRating, int totalRatings) 
{
    // 客服姓名
    QString displayName = staff.realName.isEmpty() ? staff.username : staff.realName;
    m_staffTable->setItem(row, 0, new QTableWidgetItem(displayName));
    
    // 用户名
    m_staffTable->setItem(row, 1, new QTableWidgetItem(staff.username));
    
    // 总评价数
    m_staffTable->setItem(row, 2, new QTableWidgetItem(QString::number(totalRatings)));
    
    // 平均评分
    QString avgText = totalRatings > 0 ? QString::number(avgRating, 'f', 1) : "--";
    QTableWidgetItem* avgItem = new QTableWidgetItem(avgText);
    if (totalRatings > 0) {
        if (avgRating >= 4.5) {
            avgItem->setForeground(QColor("#27AE60")); // 绿色
        } else if (avgRating <= 3.0) {
            avgItem->setForeground(QColor("#E74C3C")); // 红色
        } else {
            avgItem->setForeground(QColor("#F39C12")); // 橙色
        }
    }
    m_staffTable->setItem(row, 3, avgItem);
}

// MOC generated automatically
