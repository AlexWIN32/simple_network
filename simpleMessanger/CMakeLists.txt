
include_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/simpleMessanger)

add_executable(server server.cc MessagesProcessor.cc)
target_link_libraries(server simple_network)

#find_package(Qt4 REQUIRED QtGui QtCore)

#message (STATUS ${QT4_EXECUTABLE_COMPILE_FLAGS})
#if(NOT ${QT4_FOUND})
#    message (FATAL_ERROR "QT not foune")
#endif()

#include_directories(${QT4_INCLUDES})

#add_definitions(${QT4_DEFINITIONS})

#set(CMAKE_CXX_FLAGS "${QT4_EXECUTABLE_COMPILE_FLAGS}")

#qt4_generate_moc(addressbook.cc addressbook_moc.cc)

#add_executable(client addressbook.cc addressbook_moc.cc)
#target_link_libraries(client network ${QT4_LIBRARIES})

include_directories(/usr/include/qt4/QtCore /usr/include/qt4/QtGui /usr/include/qt4 /usr/share/qt4/mkspecs/linux-g++)
add_definitions(-D_REENTRANT -DQT_NO_DEBUG -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED -L/usr/lib/i386-linux-gnu -std=c++0x)

find_package(Qt4 4.4.3 REQUIRED QtGui QtXml)

qt4_generate_moc(DialogWindow.h DialogWindowMoc.cc)

add_executable(client client.cc DialogWindow.cc DialogWindowMoc.cc ClientMessagesProcessor.cc)
target_link_libraries(client QtGui QtCore pthread simple_network)
