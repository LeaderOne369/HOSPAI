cmake_minimum_required(VERSION 3.16)

project(HospAI VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Network)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        
        # Core files
        src/core/DatabaseManager.cpp
        src/core/AIApiClient.cpp
        
        # Common view components  
        src/views/common/UIStyleManager.cpp
        src/views/common/LoginDialog.cpp
        src/views/common/RegisterDialog.cpp
        src/views/common/ForgotPasswordDialog.cpp
        src/views/common/SettingsDialog.cpp
        
        # Patient view components
        src/views/patient/ChatWidget.cpp
        src/views/patient/RealChatWidget.cpp
        src/views/patient/SessionRatingDialog.cpp
        src/views/patient/FAQWidget.cpp
        src/views/patient/AppointmentWidget.cpp
        src/views/patient/MapWidget.cpp
        src/views/patient/PatientMainWidget.cpp
        
        # Staff view components
        src/views/staff/StaffChatManager.cpp
        src/views/staff/StatsWidget.cpp
        src/views/staff/StaffMainWidget.cpp
        src/views/staff/ConsultationWidget.cpp
        src/views/staff/KnowledgeBaseWidget.cpp
        
        # Admin view components
        src/views/admin/UserManageWidget.cpp
        src/views/admin/PatientManageWidget.cpp
        src/views/admin/QuickReplyManageWidget.cpp
        src/views/admin/StaffRatingWidget.cpp
        src/views/admin/SystemStatsWidget.cpp
        src/views/admin/SystemConfigWidget.cpp
        src/views/admin/AuditLogWidget.cpp
        src/views/admin/AdminMainWidget.cpp
)

set(PROJECT_UI_FILES
        mainwindow.ui
        src/views/common/LoginDialog.ui
        src/views/common/ForgotPasswordDialog.ui
        src/views/common/SettingsDialog.ui
)

qt_add_executable(HospAI
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    ${PROJECT_UI_FILES}
)

target_link_libraries(HospAI PRIVATE Qt6::Core Qt6::Widgets Qt6::Sql Qt6::Network)

set_target_properties(HospAI PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

# 包含目录
target_include_directories(HospAI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 编译器特定设置
if(MSVC)
    target_compile_options(HospAI PRIVATE /W4)
else()
    target_compile_options(HospAI PRIVATE -Wall -Wextra -Wpedantic)
endif()

qt_finalize_executable(HospAI)
