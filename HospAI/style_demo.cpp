#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QTabWidget>
#include <QTextEdit>
#include <QGroupBox>
#include <QListWidget>
#include <QProgressBar>
#include "src/views/common/UIStyleManager.h"

class StyleDemoWidget : public QWidget
{
public:
    StyleDemoWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setWindowTitle("HospAI 跨平台样式演示");
        setMinimumSize(800, 600);
        
        setupUI();
        
        // 应用样式
        UIStyleManager::applyGlobalStyleSheet(qApp);
    }

private:
    void setupUI()
    {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        
        // 标题
        QLabel *titleLabel = new QLabel("HospAI 跨平台样式演示", this);
        UIStyleManager::applyLabelStyle(titleLabel, "title");
        titleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);
        
        // 选项卡容器
        QTabWidget *tabWidget = new QTabWidget(this);
        
        // 第一个选项卡 - 基础控件
        QWidget *basicTab = new QWidget;
        setupBasicControlsTab(basicTab);
        tabWidget->addTab(basicTab, "🎨 基础控件");
        
        // 第二个选项卡 - 按钮样式
        QWidget *buttonTab = new QWidget;
        setupButtonStylesTab(buttonTab);
        tabWidget->addTab(buttonTab, "🔘 按钮样式");
        
        // 第三个选项卡 - 输入控件
        QWidget *inputTab = new QWidget;
        setupInputControlsTab(inputTab);
        tabWidget->addTab(inputTab, "📝 输入控件");
        
        mainLayout->addWidget(tabWidget);
        
        // 状态栏
        QLabel *statusLabel = new QLabel("跨平台样式已应用，适配 Windows、macOS 和 Linux", this);
        UIStyleManager::applyLabelStyle(statusLabel, "secondary");
        statusLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(statusLabel);
    }
    
    void setupBasicControlsTab(QWidget *parent)
    {
        QVBoxLayout *layout = new QVBoxLayout(parent);
        
        // 进度条组
        QGroupBox *progressGroup = new QGroupBox("进度指示器", parent);
        UIStyleManager::applyGroupBoxStyle(progressGroup);
        QVBoxLayout *progressLayout = new QVBoxLayout(progressGroup);
        
        QProgressBar *progress1 = new QProgressBar;
        progress1->setValue(30);
        progress1->setTextVisible(true);
        progressLayout->addWidget(progress1);
        
        QProgressBar *progress2 = new QProgressBar;
        progress2->setValue(75);
        progress2->setTextVisible(true);
        progressLayout->addWidget(progress2);
        
        layout->addWidget(progressGroup);
        
        // 列表控件
        QGroupBox *listGroup = new QGroupBox("列表控件", parent);
        UIStyleManager::applyGroupBoxStyle(listGroup);
        QVBoxLayout *listLayout = new QVBoxLayout(listGroup);
        
        QListWidget *listWidget = new QListWidget;
        listWidget->addItem("🏥 医院导航");
        listWidget->addItem("💬 智能客服");
        listWidget->addItem("📋 预约挂号");
        listWidget->addItem("📊 数据统计");
        listWidget->addItem("⚙️ 系统设置");
        listLayout->addWidget(listWidget);
        
        layout->addWidget(listGroup);
        
        layout->addStretch();
    }
    
    void setupButtonStylesTab(QWidget *parent)
    {
        QVBoxLayout *layout = new QVBoxLayout(parent);
        
        // 主要按钮
        QGroupBox *primaryGroup = new QGroupBox("主要按钮", parent);
        UIStyleManager::applyGroupBoxStyle(primaryGroup);
        QHBoxLayout *primaryLayout = new QHBoxLayout(primaryGroup);
        
        QPushButton *primaryBtn = new QPushButton("主要操作", parent);
        UIStyleManager::applyButtonStyle(primaryBtn, "primary");
        primaryLayout->addWidget(primaryBtn);
        
        QPushButton *secondaryBtn = new QPushButton("次要操作", parent);
        UIStyleManager::applyButtonStyle(secondaryBtn, "secondary");
        primaryLayout->addWidget(secondaryBtn);
        
        layout->addWidget(primaryGroup);
        
        // 状态按钮
        QGroupBox *statusGroup = new QGroupBox("状态按钮", parent);
        UIStyleManager::applyGroupBoxStyle(statusGroup);
        QHBoxLayout *statusLayout = new QHBoxLayout(statusGroup);
        
        QPushButton *successBtn = new QPushButton("✅ 成功", parent);
        UIStyleManager::applyButtonStyle(successBtn, "success");
        statusLayout->addWidget(successBtn);
        
        QPushButton *warningBtn = new QPushButton("⚠️ 警告", parent);
        UIStyleManager::applyButtonStyle(warningBtn, "warning");
        statusLayout->addWidget(warningBtn);
        
        QPushButton *errorBtn = new QPushButton("❌ 错误", parent);
        UIStyleManager::applyButtonStyle(errorBtn, "error");
        statusLayout->addWidget(errorBtn);
        
        layout->addWidget(statusGroup);
        
        layout->addStretch();
    }
    
    void setupInputControlsTab(QWidget *parent)
    {
        QVBoxLayout *layout = new QVBoxLayout(parent);
        
        // 输入框组
        QGroupBox *inputGroup = new QGroupBox("输入框", parent);
        UIStyleManager::applyGroupBoxStyle(inputGroup);
        QVBoxLayout *inputLayout = new QVBoxLayout(inputGroup);
        
        QLineEdit *nameEdit = new QLineEdit;
        nameEdit->setPlaceholderText("请输入您的姓名");
        UIStyleManager::applyLineEditStyle(nameEdit);
        inputLayout->addWidget(nameEdit);
        
        QLineEdit *phoneEdit = new QLineEdit;
        phoneEdit->setPlaceholderText("请输入手机号码");
        UIStyleManager::applyLineEditStyle(phoneEdit);
        inputLayout->addWidget(phoneEdit);
        
        layout->addWidget(inputGroup);
        
        // 文本区域
        QGroupBox *textGroup = new QGroupBox("文本区域", parent);
        UIStyleManager::applyGroupBoxStyle(textGroup);
        QVBoxLayout *textLayout = new QVBoxLayout(textGroup);
        
        QTextEdit *textEdit = new QTextEdit;
        textEdit->setPlaceholderText("请描述您的症状或问题...");
        textEdit->setMaximumHeight(150);
        UIStyleManager::applyTextEditStyle(textEdit);
        textLayout->addWidget(textEdit);
        
        layout->addWidget(textGroup);
        
        layout->addStretch();
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 应用跨平台样式
    UIStyleManager::setupFonts();
    UIStyleManager::applyGlobalStyleSheet(&app);
    
    StyleDemoWidget demo;
    demo.show();
    
    return app.exec();
}

#include "style_demo.moc" 