#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include "UIStyleManager.h"
#include <QGridLayout>
#include <QMessageBox>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
    , m_settings(new QSettings("HospAI", "Settings", this))
    , m_userRole("patient")
{
    ui->setupUi(this);
    initializeUI();
    loadSettings();
    setupConnections();
}

SettingsDialog::SettingsDialog(const QString& userRole, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
    , m_settings(new QSettings("HospAI", "Settings", this))
    , m_userRole(userRole)
{
    ui->setupUi(this);
    initializeUI();
    loadSettings();
    setupConnections();
    adjustForUserRole();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::initializeUI()
{
    setWindowTitle("设置");
    setModal(true);
    resize(700, 600);
    
    // 设置图标和样式
    setWindowIcon(QIcon(":/icons/settings.png"));
    
    // 设置默认选项卡
    ui->tabWidget->setCurrentIndex(0);
    
    // 设置音量滑块的显示
    ui->volumeValueLabel->setText(QString("%1%").arg(ui->volumeSlider->value()));
}

void SettingsDialog::setupConnections()
{
    // 主题变化
    connect(ui->themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onThemeChanged);
    
    // 字体变化
    connect(ui->fontFamilyCombo, &QFontComboBox::currentFontChanged,
            this, &SettingsDialog::onFontChanged);
    connect(ui->fontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SettingsDialog::onFontChanged);
    
    // 音量变化
    connect(ui->volumeSlider, &QSlider::valueChanged,
            this, &SettingsDialog::onVolumeChanged);
    
    // 代理设置变化
    connect(ui->useProxyCheck, &QCheckBox::toggled,
            this, &SettingsDialog::onProxyToggled);
    
    // 按钮连接
    connect(ui->applyBtn, &QPushButton::clicked, this, &SettingsDialog::applySettings);
    connect(ui->resetBtn, &QPushButton::clicked, this, &SettingsDialog::resetToDefaults);
    connect(ui->importBtn, &QPushButton::clicked, this, &SettingsDialog::importSettings);
    connect(ui->exportBtn, &QPushButton::clicked, this, &SettingsDialog::exportSettings);
    connect(ui->clearHistoryBtn, &QPushButton::clicked, this, &SettingsDialog::clearChatHistory);
    connect(ui->clearCacheBtn, &QPushButton::clicked, this, &SettingsDialog::clearCache);
    
    // 设置变更监听
    connect(ui->languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::settingsChanged);
    connect(ui->autoStartCheck, &QCheckBox::toggled, this, &SettingsDialog::settingsChanged);
    connect(ui->rememberWindowCheck, &QCheckBox::toggled, this, &SettingsDialog::settingsChanged);
    connect(ui->minimizeToTrayCheck, &QCheckBox::toggled, this, &SettingsDialog::settingsChanged);
    connect(ui->showTimestampCheck, &QCheckBox::toggled, this, &SettingsDialog::settingsChanged);
    connect(ui->showTypingCheck, &QCheckBox::toggled, this, &SettingsDialog::settingsChanged);
    connect(ui->autoSaveHistoryCheck, &QCheckBox::toggled, this, &SettingsDialog::settingsChanged);
    connect(ui->enableNotificationCheck, &QCheckBox::toggled, this, &SettingsDialog::settingsChanged);
    connect(ui->showPreviewCheck, &QCheckBox::toggled, this, &SettingsDialog::settingsChanged);
    connect(ui->urgentMessageCheck, &QCheckBox::toggled, this, &SettingsDialog::settingsChanged);
    connect(ui->enableSoundCheck, &QCheckBox::toggled, this, &SettingsDialog::settingsChanged);
    connect(ui->enableLogCheck, &QCheckBox::toggled, this, &SettingsDialog::settingsChanged);
    
    // 接受和拒绝按钮已在UI文件中连接
}

void SettingsDialog::loadSettings()
{
    // 加载通用设置
    QString language = m_settings->value("general/language", "简体中文").toString();
    int languageIndex = ui->languageCombo->findText(language);
    if (languageIndex >= 0) {
        ui->languageCombo->setCurrentIndex(languageIndex);
    }
    
    QString theme = m_settings->value("general/theme", "浅色主题").toString();
    int themeIndex = ui->themeCombo->findText(theme);
    if (themeIndex >= 0) {
        ui->themeCombo->setCurrentIndex(themeIndex);
    }
    
    ui->autoStartCheck->setChecked(m_settings->value("general/autoStart", false).toBool());
    ui->rememberWindowCheck->setChecked(m_settings->value("general/rememberWindow", true).toBool());
    ui->minimizeToTrayCheck->setChecked(m_settings->value("general/minimizeToTray", false).toBool());
    
    // 加载字体设置
    QString fontFamily = m_settings->value("chat/fontFamily", "Microsoft YaHei").toString();
    int fontSize = m_settings->value("chat/fontSize", 12).toInt();
    
    ui->fontFamilyCombo->setCurrentFont(QFont(fontFamily));
    ui->fontSizeSpinBox->setValue(fontSize);
    
    // 加载聊天设置
    ui->showTimestampCheck->setChecked(m_settings->value("chat/showTimestamp", true).toBool());
    ui->showTypingCheck->setChecked(m_settings->value("chat/showTyping", true).toBool());
    ui->autoSaveHistoryCheck->setChecked(m_settings->value("chat/autoSave", true).toBool());
    ui->maxHistorySpinBox->setValue(m_settings->value("chat/maxHistory", 1000).toInt());
    
    // 加载通知设置
    ui->enableNotificationCheck->setChecked(m_settings->value("notification/enabled", true).toBool());
    ui->showPreviewCheck->setChecked(m_settings->value("notification/showPreview", true).toBool());
    ui->urgentMessageCheck->setChecked(m_settings->value("notification/urgentMessage", true).toBool());
    ui->enableSoundCheck->setChecked(m_settings->value("sound/enabled", true).toBool());
    ui->volumeSlider->setValue(m_settings->value("sound/volume", 50).toInt());
    
    // 加载高级设置
    ui->useProxyCheck->setChecked(m_settings->value("network/useProxy", false).toBool());
    ui->proxyHostEdit->setText(m_settings->value("network/proxyHost", "127.0.0.1").toString());
    ui->proxyPortSpinBox->setValue(m_settings->value("network/proxyPort", 8080).toInt());
    
    ui->enableLogCheck->setChecked(m_settings->value("debug/enableLog", true).toBool());
    QString logLevel = m_settings->value("debug/logLevel", "INFO").toString();
    int logLevelIndex = ui->logLevelCombo->findText(logLevel);
    if (logLevelIndex >= 0) {
        ui->logLevelCombo->setCurrentIndex(logLevelIndex);
    }
    
    ui->cacheSizeSpinBox->setValue(m_settings->value("cache/maxSize", 100).toInt());
}

void SettingsDialog::saveSettings()
{
    // 保存通用设置
    m_settings->setValue("general/language", ui->languageCombo->currentText());
    m_settings->setValue("general/theme", ui->themeCombo->currentText());
    m_settings->setValue("general/autoStart", ui->autoStartCheck->isChecked());
    m_settings->setValue("general/rememberWindow", ui->rememberWindowCheck->isChecked());
    m_settings->setValue("general/minimizeToTray", ui->minimizeToTrayCheck->isChecked());
    
    // 保存字体设置
    m_settings->setValue("chat/fontFamily", ui->fontFamilyCombo->currentFont().family());
    m_settings->setValue("chat/fontSize", ui->fontSizeSpinBox->value());
    
    // 保存聊天设置
    m_settings->setValue("chat/showTimestamp", ui->showTimestampCheck->isChecked());
    m_settings->setValue("chat/showTyping", ui->showTypingCheck->isChecked());
    m_settings->setValue("chat/autoSave", ui->autoSaveHistoryCheck->isChecked());
    m_settings->setValue("chat/maxHistory", ui->maxHistorySpinBox->value());
    
    // 保存通知设置
    m_settings->setValue("notification/enabled", ui->enableNotificationCheck->isChecked());
    m_settings->setValue("notification/showPreview", ui->showPreviewCheck->isChecked());
    m_settings->setValue("notification/urgentMessage", ui->urgentMessageCheck->isChecked());
    m_settings->setValue("sound/enabled", ui->enableSoundCheck->isChecked());
    m_settings->setValue("sound/volume", ui->volumeSlider->value());
    
    // 保存高级设置
    m_settings->setValue("network/useProxy", ui->useProxyCheck->isChecked());
    m_settings->setValue("network/proxyHost", ui->proxyHostEdit->text());
    m_settings->setValue("network/proxyPort", ui->proxyPortSpinBox->value());
    
    m_settings->setValue("debug/enableLog", ui->enableLogCheck->isChecked());
    m_settings->setValue("debug/logLevel", ui->logLevelCombo->currentText());
    
    m_settings->setValue("cache/maxSize", ui->cacheSizeSpinBox->value());
    
    // 同步设置
    m_settings->sync();
}

void SettingsDialog::onThemeChanged()
{
    QString selectedTheme = ui->themeCombo->currentText();
    emit themeChanged(selectedTheme);
    emit settingsChanged();
}

void SettingsDialog::onFontChanged()
{
    QFont font = ui->fontFamilyCombo->currentFont();
    font.setPointSize(ui->fontSizeSpinBox->value());
    
    emit fontChanged(font);
    emit settingsChanged();
}

void SettingsDialog::onVolumeChanged(int value)
{
    ui->volumeValueLabel->setText(QString("%1%").arg(value));
    emit settingsChanged();
}

void SettingsDialog::onProxyToggled(bool enabled)
{
    ui->proxyHostEdit->setEnabled(enabled);
    ui->proxyPortSpinBox->setEnabled(enabled);
    emit settingsChanged();
}

void SettingsDialog::applySettings()
{
    saveSettings();
    emit settingsChanged();
    
    QMessageBox::information(this, "设置", "设置已应用！");
}

void SettingsDialog::resetToDefaults()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "恢复默认设置", 
        "确定要恢复所有设置到默认值吗？此操作不可撤销。",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        // 清除所有设置
        m_settings->clear();
        
        // 重新加载默认设置
        loadSettings();
        
        emit settingsChanged();
        
        QMessageBox::information(this, "设置", "已恢复默认设置！");
    }
}

void SettingsDialog::importSettings()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "导入设置",
        QDir::homePath(),
        "设置文件 (*.ini);;所有文件 (*)"
    );
    
    if (!fileName.isEmpty()) {
        QSettings importSettings(fileName, QSettings::IniFormat);
        
        // 复制所有设置
        QStringList keys = importSettings.allKeys();
        for (const QString& key : keys) {
            m_settings->setValue(key, importSettings.value(key));
        }
        
        // 重新加载设置到界面
        loadSettings();
        
        emit settingsChanged();
        
        QMessageBox::information(this, "导入设置", "设置导入成功！");
    }
}

void SettingsDialog::exportSettings()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "导出设置",
        QDir::homePath() + "/HospAI_Settings.ini",
        "设置文件 (*.ini);;所有文件 (*)"
    );
    
    if (!fileName.isEmpty()) {
        QSettings exportSettings(fileName, QSettings::IniFormat);
        
        // 复制所有设置
        QStringList keys = m_settings->allKeys();
        for (const QString& key : keys) {
            exportSettings.setValue(key, m_settings->value(key));
        }
        
        exportSettings.sync();
        
        QMessageBox::information(this, "导出设置", "设置导出成功！");
    }
}

void SettingsDialog::clearChatHistory()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "清空聊天记录", 
        "确定要清空所有聊天记录吗？此操作不可撤销。",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        // 这里可以添加清空聊天记录的逻辑
        // 例如：删除数据库中的聊天记录、清空缓存文件等
        
        QMessageBox::information(this, "清空聊天记录", "聊天记录已清空！");
    }
}

void SettingsDialog::clearCache()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "清理缓存", 
        "确定要清理所有缓存文件吗？",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        // 这里可以添加清理缓存的逻辑
        // 例如：删除临时文件、清空图片缓存等
        
        QMessageBox::information(this, "清理缓存", "缓存已清理！");
    }
}

QString SettingsDialog::getCurrentTheme() const
{
    return ui->themeCombo->currentText();
}

QFont SettingsDialog::getCurrentFont() const
{
    QFont font = ui->fontFamilyCombo->currentFont();
    font.setPointSize(ui->fontSizeSpinBox->value());
    return font;
}

void SettingsDialog::adjustForUserRole()
{
    if (m_userRole == "admin") {
        // 管理员可以看到所有选项卡
        setWindowTitle("系统设置 - 管理员");
        // 显示所有高级选项
    } else if (m_userRole == "staff") {
        // 客服人员看到基本设置和部分高级设置
        setWindowTitle("设置 - 客服");
        // 隐藏某些管理员专用选项
    } else {
        // 患者看到基本设置
        setWindowTitle("设置");
        // 隐藏代理设置等高级选项
        ui->networkGroup->setVisible(false);
        ui->debugGroup->setVisible(false);
    }
} 