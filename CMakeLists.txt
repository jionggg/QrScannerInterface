cmake_minimum_required(VERSION 3.16)

project(ZeissApp1 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt 查找
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Concurrent)
# ✅ OpenCV 查找：用你提供的路径
set(OpenCV_DIR "C:/opencv/build/x64/vc15/lib")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# 源文件列表
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

# 可执行文件构建
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ZeissApp1
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(ZeissApp1 SHARED ${PROJECT_SOURCES})
    else()
        add_executable(ZeissApp1 ${PROJECT_SOURCES})
    endif()
endif()

# 链接 Qt + OpenCV
target_link_libraries(ZeissApp1
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    ${OpenCV_LIBS}
)

# 可选的 macOS/iOS 打包设置
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ZeissApp1)
endif()
set_target_properties(ZeissApp1 PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# 安装路径配置
include(GNUInstallDirs)
install(TARGETS ZeissApp1
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Qt6 专用 finalize
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ZeissApp1)
endif()
