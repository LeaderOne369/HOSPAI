#ifndef STAFFRATINGWIDGET_H
#define STAFFRATINGWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QDateEdit>
#include <QSpinBox>
#include <QDialog>
#include <QFormLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QTextEdit>
#include <QProgressBar>
#include "../../core/DatabaseManager.h"

class StaffRatingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StaffRatingWidget(QWidget *parent = nullptr);
    void setDatabaseManager(DatabaseManager* dbManager);

private slots:
    void onRefreshRatings();
    void onSearchRatings();
    void onRatingSelectionChanged();
    void onExportRatings();
    void onViewRatingDetails();
    void onShowStaffStats();
    void onDateRangeChanged();

private:
    void setupUI();
    void setupStatsPanel();
    void setupRatingChart();
    void loadRatings();
    void addRatingToTable(int row, const SessionRating& rating, const QString& patientName, const QString& staffName);
    void updateStatsPanel(const QList<SessionRating>& ratings);
    void updateRatingChart();
    void showRatingDetailsDialog(const SessionRating& rating);

private:
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_topLayout;
    
    // 左侧面板 - 统计信息
    QWidget* m_leftPanel;
    QVBoxLayout* m_leftLayout;
    QGroupBox* m_statsGroup;
    QLabel* m_totalRatingsLabel;
    QLabel* m_avgRatingLabel;
    QLabel* m_bestStaffLabel;
    QLabel* m_worstStaffLabel;
    QProgressBar* m_satisfaction5Star;
    QProgressBar* m_satisfaction4Star;
    QProgressBar* m_satisfaction3Star;
    QProgressBar* m_satisfaction2Star;
    QProgressBar* m_satisfaction1Star;
    
    // 右侧面板 - 评价列表
    QWidget* m_rightPanel;
    QVBoxLayout* m_rightLayout;
    
    // 顶部工具栏
    QHBoxLayout* m_toolLayout;
    QPushButton* m_btnRefresh;
    QPushButton* m_btnExport;
    QPushButton* m_btnViewDetails;
    QPushButton* m_btnStaffStats;
    
    // 搜索区域
    QGroupBox* m_searchGroup;
    QComboBox* m_staffFilter;
    QComboBox* m_ratingFilter;
    QDateEdit* m_dateFromEdit;
    QDateEdit* m_dateToEdit;
    QLineEdit* m_searchEdit;
    QPushButton* m_btnSearch;
    
    // 评价列表
    QTableWidget* m_ratingTable;
    
    // 统计信息
    QLabel* m_statsLabel;
    
    // 数据
    DatabaseManager* m_dbManager;
    QList<SessionRating> m_ratings;
    QMap<int, UserInfo> m_staffMap; // 客服信息缓存
};

// 评价详情对话框
class RatingDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RatingDetailsDialog(const SessionRating& rating, const UserInfo& staff, const UserInfo& patient, QWidget *parent = nullptr);

private:
    void setupUI();

private:
    SessionRating m_rating;
    UserInfo m_staff;
    UserInfo m_patient;
    
    QVBoxLayout* m_mainLayout;
    QLabel* m_patientLabel;
    QLabel* m_staffLabel;
    QLabel* m_sessionLabel;
    QLabel* m_ratingLabel;
    QTextEdit* m_commentEdit;
    QLabel* m_timeLabel;
    QPushButton* m_closeButton;
};

// 客服统计对话框
class StaffStatsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StaffStatsDialog(DatabaseManager* dbManager, QWidget *parent = nullptr);

private slots:
    void onRefreshStats();
    void onStaffSelectionChanged();

private:
    void setupUI();
    void loadStaffStats();
    void addStaffToTable(int row, const UserInfo& staff, double avgRating, int totalRatings);

private:
    DatabaseManager* m_dbManager;
    QVBoxLayout* m_mainLayout;
    QTableWidget* m_staffTable;
    QLabel* m_statsLabel;
    QPushButton* m_refreshButton;
    QPushButton* m_closeButton;
};

#endif // STAFFRATINGWIDGET_H 