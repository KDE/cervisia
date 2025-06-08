find_package(KF${KF_MAJOR_VERSION}DBusAddons)

set(cvsservice_bin_SRCS
   main.cpp 
   cvsservice.cpp 
   cvsjob.cpp 
   repository.cpp 
   sshagent.cpp 
   cvsserviceutils.cpp 
   cvsloginjob.cpp
   cvsservice.h
   cvsjob.h
   repository.h
   sshagent.h
   cvsserviceutils.h
   cvsloginjob.h
   ../debug.cpp)

qt_add_dbus_adaptor(cvsservice_bin_SRCS org.kde.cervisia5.cvsservice.xml cvsservice.h CvsService)

qt_add_dbus_adaptor(cvsservice_bin_SRCS org.kde.cervisia5.cvsloginjob.xml cvsloginjob.h CvsLoginJob)

qt_add_dbus_adaptor(cvsservice_bin_SRCS org.kde.cervisia5.repository.xml repository.h Repository)

qt_add_dbus_adaptor(cvsservice_bin_SRCS org.kde.cervisia5.cvsjob.xml cvsjob.h CvsJob)



add_executable(cvsservice_bin ${cvsservice_bin_SRCS})
ecm_mark_nongui_executable(cvsservice_bin)
set_target_properties(cvsservice_bin PROPERTIES OUTPUT_NAME cvsservice5)

target_link_libraries(cvsservice_bin Qt::Widgets Qt::DBus KF${KF_MAJOR_VERSION}::KIOCore KF${KF_MAJOR_VERSION}::I18n KF${KF_MAJOR_VERSION}::WidgetsAddons KF${KF_MAJOR_VERSION}::Su KF${KF_MAJOR_VERSION}::DBusAddons KF${KF_MAJOR_VERSION}::ConfigCore)
if (QT_MAJOR_VERSION STREQUAL "6")
    target_link_libraries(cvsservice_bin Qt::Core5Compat)
endif()

install(TARGETS cvsservice_bin ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

########### next target ###############

add_executable(cvsaskpass cvsaskpass.cpp)

target_link_libraries(cvsaskpass Qt::Widgets KF${KF_MAJOR_VERSION}::WidgetsAddons KF${KF_MAJOR_VERSION}::CoreAddons KF${KF_MAJOR_VERSION}::I18n)
if (QT_MAJOR_VERSION STREQUAL "6")
    target_link_libraries(cvsaskpass Qt::Core5Compat)
endif()

install(TARGETS cvsaskpass  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

if (QT_MAJOR_VERSION STREQUAL "5")
    install( FILES  org.kde.cvsservice5.desktop  DESTINATION  ${KDE_INSTALL_KSERVICESDIR} )
endif()

install( FILES org.kde.cervisia5.cvsjob.xml
               org.kde.cervisia5.cvsservice.xml
               org.kde.cervisia5.repository.xml
               org.kde.cervisia5.cvsloginjob.xml
         DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR} )
