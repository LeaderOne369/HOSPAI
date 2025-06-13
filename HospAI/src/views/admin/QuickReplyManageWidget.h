#ifndef QUICKREPLYMANAGEWIDGET_H
#define QUICKREPLYMANAGEWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QTextEdit>
#include <QSpinBox>
#include <QDialog>
#include <QFormLayout>
#include <QHeaderView>
#include <QMessageBox>
#include "../../core/DatabaseManager.h"

class QuickReplyManageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QuickReplyManageWidget(QWidget *parent = nullptr);
    void setDatabaseManager(DatabaseManager* dbManager);

private slots:
    void onAddReply();
    void onEditReply();
    void onDeleteReply();
    void onRefreshReplies();
    void onSearchReplies();
    void onReplySelectionChanged();
    void onExportReplies();
    void onImportReplies();
    void onMoveUp();
    void onMoveDown();

private:
    void setupUI();
    void loadQuickReplies();
    void addReplyToTable(int row, const QuickReply& reply);
    void showReplyDialog(const QuickReply& reply = QuickReply());
    void updateSortOrder();

private:
    QVBoxLayout* m_mainLayout;
    
    // 顶部工具栏
    QHBoxLayout* m_toolLayout;
    QPushButton* m_btnAdd;
    QPushButton* m_btnEdit;
    QPushButton* m_btnDelete;
    QPushButton* m_btnRefresh;
    QPushButton* m_btnExport;
    QPushButton* m_btnImport;
    QPushButton* m_btnMoveUp;
    QPushButton* m_btnMoveDown;
    
    // 搜索区域
    QGroupBox* m_searchGroup;
    QLineEdit* m_searchEdit;
    QComboBox* m_categoryFilter;
    QComboBox* m_statusFilter;
    QPushButton* m_btnSearch;
    
    // 快捷回复列表
    QTableWidget* m_replyTable;
    
    // 统计信息
    QLabel* m_statsLabel;
    
    // 数据
    DatabaseManager* m_dbManager;
    QList<QuickReply> m_replies;
};

// 快捷回复编辑对话框
class QuickReplyEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QuickReplyEditDialog(const QuickReply& reply = QuickReply(), QWidget *parent = nullptr);
    QuickReply getQuickReply() const;

private slots:
    void onAccept();
    void onPreview();

private:
    void setupUI();

private:
    QFormLayout* m_formLayout;
    QLineEdit* m_titleEdit;
    QTextEdit* m_contentEdit;
    QComboBox* m_categoryCombo;
    QSpinBox* m_sortSpin;
    QComboBox* m_statusCombo;
    
    QPushButton* m_previewButton;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    
    QuickReply m_originalReply;
    bool m_isEditing;
};

#endif // QUICKREPLYMANAGEWIDGET_H 