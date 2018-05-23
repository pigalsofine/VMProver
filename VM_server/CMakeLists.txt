cmake_minimum_required(VERSION 3.5.1)
project(VM_server)

set(CMAKE_CXX_STANDARD 11)

add_executable(VM_server main.cpp server.cpp definition.h Features.h Features.cpp cmp_file.cpp cmp_file.h
        CustomArtProv.cpp CustomArtProv.h FileTree.cpp FileTree.h ModuleList.cpp ModuleList.h PanelFile.cpp
        PanelFile.h PanelModule.cpp PanelModule.h PanelProcess.cpp PanelProcess.h PanelVM.cpp PanelVM.h
        PanelWelcome.cpp PanelWelcome.h ProcessTree.cpp ProcessTree.h Profile.cpp Profile.h ProfileMain.cpp
        ProfileMain.h SettingMain.cpp SettingMain.h SettingsPanel.cpp SettingsPanel.h SizeReportCtrl.cpp
        SizeReportCtrl.h VMdefendor.cpp VMdefendor.h VMdefendorFrame.cpp VMdefendorFrame.h VMList.cpp
        VMList.cpp VMList.h VMStatusBar.cpp VMStatusBar.h WorkerProcess.cpp WorkerProcess.h getTxt.h getTxt.cpp
        process_list.h process_list.cpp wrap.cpp wrap.h)

FIND_PACKAGE(wxWidgets REQUIRED)

IF(wxWidgets_FOUND)
    MESSAGE(${wxWidgets_USE_FILE})
    INCLUDE(${wxWidgets_USE_FILE})
    MESSAGE(${wxWidgets_LIBRARIES})
    TARGET_LINK_LIBRARIES(VM_server -I/usr/local/lib/wx/include/gtk3-unicode-static-3.0 -I/usr/local/include/wx-3.0 -D_FILE_OFFSET_BITS=64 -D__WXGTK__ -pthread -L/usr/local/lib -pthread /usr/local/lib/libwx_gtk3u_aui-3.0.a /usr/local/lib/libwx_gtk3u_xrc-3.0.a /usr/local/lib/libwx_gtk3u_qa-3.0.a /usr/local/lib/libwx_baseu_net-3.0.a /usr/local/lib/libwx_gtk3u_html-3.0.a /usr/local/lib/libwx_gtk3u_adv-3.0.a /usr/local/lib/libwx_gtk3u_core-3.0.a /usr/local/lib/libwx_baseu_xml-3.0.a /usr/local/lib/libwx_baseu-3.0.a -lgthread-2.0 -pthread -lX11 -lXxf86vm -lSM -lgtk-3 -lgdk-3 -lpangocairo-1.0 -lpango-1.0 -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lpng -lexpat -lwxregexu-3.0 -lwxtiff-3.0 -lwxjpeg-3.0 -lz -ldl -lm  -lcapstone)
ENDIF(wxWidgets_FOUND)