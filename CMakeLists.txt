
cmake_minimum_required(VERSION 3.16)

project(TickerLite VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt5组件
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network PrintSupport Sql)

# 设置Qt5的MOC、UIC和RCC自动处理
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 添加包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/third_party/qcustomplot
)

# 收集源文件
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")
file(GLOB_RECURSE QCUSTOMPLOT_SOURCE ${CMAKE_SOURCE_DIR}"/src/qcustomplot/qcustomplot.*")

# 添加资源文件
qt5_add_resources(RESOURCES resources/resources.qrc)

# 添加可执行文件
add_executable(TickerLite
    ${SOURCES}
    ${HEADERS}
    ${QCUSTOMPLOT_SOURCE}
    ${RESOURCES}
)

# 链接Qt5库
target_link_libraries(TickerLite
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
    Qt5::PrintSupport
    Qt5::Sql
)

# 设置Windows子系统
if(WIN32)
    set_target_properties(TickerLite PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
