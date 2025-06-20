#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QSlider>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFontComboBox>
#include <QFormLayout>
#include <QSpacerItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QFont>

QT_BEGIN_NAMESPACE
namespace Ui { class SettingsDialog; }
QT_END_NAMESPACE

/**
 * @brief 设置对话框类
 * 
 * 提供系统各种设置选项，包括：
 * - 通用设置：语言、主题、启动选项
 * - 聊天设置：字体、消息显示选项
 * - 通知设置：桌面通知、声音设置
 * - 高级设置：网络代理、调试选项
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口
     */
    explicit SettingsDialog(QWidget *parent = nullptr);
    
    /**
     * @brief 构造函数（带用户角色）
     * @param userRole 用户角色（patient/staff/admin）
     * @param parent 父窗口
     */
    explicit SettingsDialog(const QString& userRole, QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~SettingsDialog();

    /**
     * @brief 获取当前主题
     * @return 主题名称
     */
    QString getCurrentTheme() const;
    
    /**
     * @brief 获取当前字体
     * @return 字体对象
     */
    QFont getCurrentFont() const;

signals:
    /**
     * @brief 设置发生变化的信号
     */
    void settingsChanged();
    
    /**
     * @brief 字体变化的信号
     * @param font 新字体
     */
    void fontChanged(const QFont& font);
    
    /**
     * @brief 主题变化的信号
     * @param theme 新主题名称
     */
    void themeChanged(const QString& theme);

public slots:
    /**
     * @brief 应用设置
     */
    void applySettings();
    
    /**
     * @brief 恢复默认设置
     */
    void resetToDefaults();

private slots:
    /**
     * @brief 主题变化槽函数
     */
    void onThemeChanged();
    
    /**
     * @brief 字体变化槽函数
     */
    void onFontChanged();
    
    /**
     * @brief 音量变化槽函数
     */
    void onVolumeChanged(int value);
    
    /**
     * @brief 代理设置变化槽函数
     */
    void onProxyToggled(bool enabled);
    
    /**
     * @brief 导入设置
     */
    void importSettings();
    
    /**
     * @brief 导出设置
     */
    void exportSettings();
    
    /**
     * @brief 清空聊天记录
     */
    void clearChatHistory();
    
    /**
     * @brief 清理缓存
     */
    void clearCache();

private:
    /**
     * @brief 初始化UI界面
     */
    void initializeUI();
    
    /**
     * @brief 设置信号连接
     */
    void setupConnections();
    
    /**
     * @brief 加载当前设置
     */
    void loadSettings();
    
    /**
     * @brief 保存设置
     */
    void saveSettings();
    
    /**
     * @brief 根据用户角色调整界面
     */
    void adjustForUserRole();

private:
    Ui::SettingsDialog *ui;          // UI对象
    QSettings* m_settings;           // 设置存储
    QString m_userRole;              // 用户角色
};

#endif // SETTINGSDIALOG_H 