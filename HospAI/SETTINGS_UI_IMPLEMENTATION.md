# HospAI 设置功能 UI 文件实现

## 概述

根据用户需求，我们将 HospAI 系统的设置功能从纯代码实现改为使用 Qt Designer 的 .ui 文件来设计界面。这种方式具有以下优势：

- 🎨 **可视化设计**: 使用 Qt Designer 进行直观的界面设计
- 🔧 **易于维护**: UI 和逻辑分离，便于修改界面布局
- 📐 **布局精确**: 通过可视化工具精确控制控件位置和大小
- 🔄 **快速原型**: 快速创建和测试不同的界面设计

## 实现架构

### 文件结构
```
src/views/common/
├── SettingsDialog.h          # 设置对话框头文件
├── SettingsDialog.cpp        # 设置对话框实现
└── SettingsDialog.ui         # Qt Designer UI 文件
```

### 核心组件

#### 1. UI 文件 (SettingsDialog.ui)
- **文件格式**: XML 格式的 Qt Designer 文件
- **窗口大小**: 700x600 像素
- **布局方式**: 主垂直布局 + 选项卡布局
- **选项卡数量**: 4个（通用、聊天、通知、高级）

#### 2. 头文件 (SettingsDialog.h)
```cpp
QT_BEGIN_NAMESPACE
namespace Ui { class SettingsDialog; }
QT_END_NAMESPACE

class SettingsDialog : public QDialog
{
    Q_OBJECT

private:
    Ui::SettingsDialog *ui;          // UI对象
    QSettings* m_settings;           // 设置存储
    QString m_userRole;              // 用户角色
};
```

#### 3. 实现文件 (SettingsDialog.cpp)
- **UI 初始化**: `ui->setupUi(this)`
- **信号连接**: 将 UI 控件信号连接到槽函数
- **设置加载**: 从 QSettings 加载配置到 UI 控件
- **设置保存**: 将 UI 控件值保存到 QSettings

## 功能特性

### 🎛️ 四个设置选项卡

#### 1. ⚙️ 通用设置
- **语言设置**: 简体中文、English、繁體中文
- **主题设置**: 浅色主题、深色主题、护眼主题、跟随系统
- **启动选项**: 
  - 开机自动启动
  - 记住窗口位置和大小
  - 最小化到系统托盘

#### 2. 💬 聊天设置
- **字体设置**: 
  - 字体选择器 (QFontComboBox)
  - 字体大小 (8-36pt)
- **消息设置**:
  - 显示消息时间戳
  - 显示正在输入提示
  - 自动保存聊天记录
- **历史记录**:
  - 最大保存条数 (100-10000)
  - 清空聊天记录按钮

#### 3. 🔔 通知设置
- **桌面通知**:
  - 启用桌面通知
  - 显示消息预览
  - 紧急消息单独提醒
- **声音设置**:
  - 启用消息提示音
  - 音量控制滑块 (0-100%)

#### 4. 🔧 高级设置
- **网络设置**:
  - 使用代理服务器
  - 代理地址和端口设置
- **调试设置**:
  - 启用日志记录
  - 日志级别选择 (ERROR/WARNING/INFO/DEBUG)
- **缓存管理**:
  - 缓存大小限制 (10-1000MB)
  - 清理缓存按钮

### 🔗 底部操作按钮
- **导入设置**: 从 .ini 文件导入配置
- **导出设置**: 导出配置到 .ini 文件
- **恢复默认**: 重置所有设置到默认值
- **确定**: 保存并关闭对话框
- **取消**: 不保存直接关闭
- **应用**: 立即应用设置

## 技术实现细节

### UI 连接机制
```cpp
// 在 UI 文件中定义的连接
<connections>
  <connection>
   <sender>useProxyCheck</sender>
   <signal>toggled(bool)</signal>
   <receiver>proxyHostEdit</receiver>
   <slot>setEnabled(bool)</slot>
  </connection>
</connections>

// 在 C++ 代码中定义的连接
connect(ui->volumeSlider, &QSlider::valueChanged,
        this, &SettingsDialog::onVolumeChanged);
```

### 设置持久化
```cpp
// 保存设置
m_settings->setValue("general/theme", ui->themeCombo->currentText());
m_settings->setValue("chat/fontFamily", ui->fontFamilyCombo->currentFont().family());

// 加载设置
QString theme = m_settings->value("general/theme", "浅色主题").toString();
ui->themeCombo->setCurrentText(theme);
```

### 用户角色适配
```cpp
void SettingsDialog::adjustForUserRole()
{
    if (m_userRole == "patient") {
        // 患者只能看到基本设置
        ui->networkGroup->setVisible(false);
        ui->debugGroup->setVisible(false);
    }
}
```

## 信号与槽

### 主要信号
- `settingsChanged()`: 设置发生变化
- `themeChanged(const QString& theme)`: 主题变化
- `fontChanged(const QFont& font)`: 字体变化

### 主要槽函数
- `applySettings()`: 应用设置
- `resetToDefaults()`: 恢复默认值
- `importSettings()`: 导入设置
- `exportSettings()`: 导出设置
- `onThemeChanged()`: 主题变化处理
- `onFontChanged()`: 字体变化处理
- `onVolumeChanged(int)`: 音量变化处理

## 编译集成

### CMakeLists.txt 配置
```cmake
set(CMAKE_AUTOUIC ON)  # 启用 UIC 自动处理

set(PROJECT_SOURCES
    # ... 其他源文件 ...
    src/views/common/SettingsDialog.cpp
)
```

### 自动生成的头文件
编译时 Qt 的 UIC 工具会自动生成：
```
build/HospAI_autogen/include/ui_SettingsDialog.h
```

## 使用方法

### 在其他类中调用
```cpp
#include "SettingsDialog.h"

void SomeWidget::openSettings()
{
    SettingsDialog dialog("patient", this);
    
    // 连接信号处理设置变化
    connect(&dialog, &SettingsDialog::themeChanged,
            this, &SomeWidget::applyNewTheme);
    
    if (dialog.exec() == QDialog::Accepted) {
        // 用户点击了确定按钮
        loadUserSettings();
    }
}
```

## 优势总结

### ✅ 开发效率
- 可视化设计减少了布局代码编写
- UI 文件可以独立修改，无需重新编译逻辑代码
- Qt Designer 提供实时预览功能

### ✅ 维护便利
- UI 和业务逻辑清晰分离
- 设计师可以独立修改界面不影响功能
- 支持国际化和主题切换

### ✅ 扩展性强
- 新增控件只需在 UI 文件中拖拽
- 支持多种布局管理器
- 便于实现响应式设计

### ✅ 专业外观
- 标准的 Qt 控件样式
- 支持样式表定制
- 自动适配系统主题

## 结论

通过使用 Qt Designer 的 .ui 文件，我们成功地将 HospAI 的设置功能从纯代码实现转换为可视化设计。这种方式不仅提高了开发效率，还为后续的界面优化和功能扩展提供了更好的基础。新的设置系统支持完整的配置管理、用户角色适配和设置导入导出功能，为用户提供了更加专业和易用的配置体验。 