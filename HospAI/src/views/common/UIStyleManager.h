#ifndef UISTYLEMANAGER_H
#define UISTYLEMANAGER_H

#include <QObject>
#include <QString>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGroupBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QScrollArea>
#include <QApplication>

class UIStyleManager : public QObject
{
    Q_OBJECT

public:
    explicit UIStyleManager(QObject *parent = nullptr);
    ~UIStyleManager();

    // 应用全局样式表
    static void applyGlobalStyleSheet(QApplication *app);
    
    // 基础颜色方案
    struct ColorScheme {
        QString primary = "#3498db";        // 主色调 - 柔和蓝色
        QString primaryDark = "#2980b9";    // 深蓝色
        QString secondary = "#95a5a6";      // 次要色 - 灰色
        QString background = "#ecf0f1";     // 背景色 - 浅灰
        QString surface = "#ffffff";       // 表面色 - 白色
        QString text = "#2c3e50";          // 文字色 - 深灰蓝
        QString textSecondary = "#7f8c8d";  // 次要文字色
        QString success = "#27ae60";       // 成功色 - 绿色
        QString warning = "#f39c12";       // 警告色 - 橙色
        QString error = "#e74c3c";         // 错误色 - 红色
        QString border = "#bdc3c7";        // 边框色
        QString shadow = "rgba(0, 0, 0, 0.1)"; // 阴影色
    };

    static ColorScheme colors;

    // 控件样式应用方法
    static void applyButtonStyle(QPushButton *button, const QString &variant = "primary");
    static void applyLabelStyle(QLabel *label, const QString &variant = "normal");
    static void applyFrameStyle(QFrame *frame, bool withShadow = true);
    static void applyGroupBoxStyle(QGroupBox *groupBox);
    static void applyLineEditStyle(QLineEdit *lineEdit);
    static void applyTextEditStyle(QTextEdit *textEdit);
    static void applyScrollAreaStyle(QScrollArea *scrollArea);
    
    // 阴影效果
    static QGraphicsDropShadowEffect* createShadowEffect(QWidget *parent = nullptr);
    
    // 布局辅助
    static void applyContainerSpacing(QWidget *container);
    
    // 字体设置
    static void setupFonts();

private:
    // 获取样式表字符串
    static QString getGlobalStyleSheet();
    static QString getButtonStyleSheet(const QString &variant);
    static QString getLabelStyleSheet(const QString &variant);
    static QString getFrameStyleSheet();
    static QString getGroupBoxStyleSheet();
    static QString getLineEditStyleSheet();
    static QString getTextEditStyleSheet();
    static QString getScrollAreaStyleSheet();
};

#endif // UISTYLEMANAGER_H 