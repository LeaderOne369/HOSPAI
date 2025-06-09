cmake_minimum_required(VERSION 3.16)

project(HospAI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        
        # Core files
        src/core/UserRole.h
        src/core/ChatStorage.h
        src/core/ChatStorage.cpp
        
        # Common view components  
        src/views/common/BaseWindow.h
        src/views/common/BaseWindow.cpp
        src/views/common/SideMenuWidget.h
        src/views/common/SideMenuWidget.cpp
        src/views/common/TitleBarWidget.h
        src/views/common/TitleBarWidget.cpp
        src/views/common/RoleSelector.h
        src/views/common/RoleSelector.cpp
        src/views/common/HospitalNavigationWidget.h
        src/views/common/HospitalNavigationWidget.cpp
        src/views/common/UIStyleManager.h
        src/views/common/UIStyleManager.cpp
        src/views/common/ExampleUsageWidget.h
        src/views/common/ExampleUsageWidget.cpp
        
        # Patient view components
        src/views/patient/PatientWindow.h
        src/views/patient/PatientWindow.cpp
        src/views/patient/ChatWidget.h
        src/views/patient/ChatWidget.cpp
        src/views/patient/FAQWidget.h
        src/views/patient/FAQWidget.cpp
        src/views/patient/MapWidget.h
        src/views/patient/MapWidget.cpp
        
        # Staff view components
        src/views/staff/StaffWindow.h
        src/views/staff/StaffWindow.cpp
        src/views/staff/RecordWidget.h
        src/views/staff/RecordWidget.cpp
        src/views/staff/StatsWidget.h
        src/views/staff/StatsWidget.cpp
        src/views/staff/ManualChatWidget.h
        src/views/staff/ManualChatWidget.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(HospAI
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET HospAI APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(HospAI SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(HospAI
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(HospAI PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.HospAI)
endif()
set_target_properties(HospAI PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS HospAI
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(HospAI)
endif()
