cmake_minimum_required(VERSION 3.5)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

project(rrcc VERSION 0.8.2 LANGUAGES C CXX)

include(GNUInstallDirs)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
#set(CMAKE_AUTOUIC ON)

find_package(Qt5Core CONFIG REQUIRED)
find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5Network CONFIG REQUIRED)
find_package(Qt5Concurrent CONFIG REQUIRED)
find_package(Qt5LinguistTools CONFIG REQUIRED)

find_package(libssh CONFIG REQUIRED)
find_package(libarchive REQUIRED)

set(rrcc_SRCS
    mainwindow.cpp
    history.cpp
    timer.cpp
    setup.cpp
    logger.cpp
    upload.cpp
    packager.cpp
    unpackager.cpp
    converter.cpp
    about.cpp
    zones.cpp
    installer.cpp
    uninstaller.cpp
    update.cpp
    download.cpp)

set(qarchive_SRCS
    ext/qarchive/qarchive.cpp)

set(qaes_SRCS
    ext/qaes/qaesencryption.cpp)

set(qsshsocket_SRCS
    ext/qsshsocket/qsshsocket.cpp)

set(ccrypt_SRCS
    ext/ccrypt/ccryptlib.c
    ext/ccrypt/rijndael.c
    ext/ccrypt/tables.c
    ext/ccrypt/platform.c)

qt5_wrap_ui(rrcc_FORMS
    res/ui/about.ui
    res/ui/converter.ui
    res/ui/download.ui
    res/ui/history.ui
    res/ui/installer.ui
    res/ui/logger.ui
    res/ui/mainwindow.ui
    res/ui/packager.ui
    res/ui/setup.ui
    res/ui/timer.ui
    res/ui/uninstaller.ui
    res/ui/unpackager.ui
    res/ui/update.ui
    res/ui/upload.ui
    res/ui/zones.ui)

qt5_add_resources(rrcc_RSCS
    res/rrcc.qrc)

add_executable(rrcc
               ${rrcc_SRCS}
               ${qarchive_SRCS}
               ${qaes_SRCS}
               ${qsshsocket_SRCS}
               ${ccrypt_SRCS}
               ${rrcc_FORMS}
               ${rrcc_RSCS})

target_include_directories(rrcc
                           PRIVATE
                               ext/ccrypt
                               ext/qaes
                               ext/qarchive
                               ext/qsshsocket
                               ${LIBSSH_INCLUDE_DIR}
                               ${LIBARCHIVE_INCLUDE_DIR})

target_link_libraries(rrcc
    Qt5::Core
    Qt5::Widgets
    Qt5::Concurrent
    Qt5::Network
    ${LIBSSH_LIBRARIES}
    ${LIBARCHIVE_LIBRARIES})

install(TARGETS
            rrcc
        ARCHIVE DESTINATION
            ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION
            ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION
            ${CMAKE_INSTALL_BINDIR}
        COMPONENT ${PROJECT_NAME})

add_subdirectory(lng)
add_subdirectory(res)
