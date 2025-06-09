#include "src/views/common/RoleSelector.h"
#include "src/views/patient/PatientWindow.h"
#include "src/views/staff/StaffWindow.h"
#include "mainwindow.h"
#include "src/views/common/ExampleUsageWidget.h"
#include "src/views/common/UIStyleManager.h"
#include <QApplication>
#include <QStyleFactory>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("HospAI");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Hospital AI Team");
    
    // 应用全局样式
    UIStyleManager::applyGlobalStyleSheet(&app);
    
    // 创建并显示示例程序
    ExampleUsageWidget *example = new ExampleUsageWidget();
    example->setWindowTitle("🏥 HospAI - 功能演示");
    example->resize(1200, 800);
    example->show();

    return app.exec();
}
