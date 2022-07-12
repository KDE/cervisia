find_package(KF5DBusAddons)

set(cvsservice_bin_KDEINIT_SRCS 
   main.cpp 
   cvsservice.cpp 
   cvsjob.cpp 
   repository.cpp 
   sshagent.cpp 
   cvsserviceutils.cpp 
   cvsloginjob.cpp
   ../debug.cpp)

qt_add_dbus_adaptor(cvsservice_bin_KDEINIT_SRCS org.kde.cervisia5.cvsservice.xml cvsservice.h CvsService)

qt_add_dbus_adaptor(cvsservice_bin_KDEINIT_SRCS org.kde.cervisia5.cvsloginjob.xml cvsloginjob.h CvsLoginJob)

qt_add_dbus_adaptor(cvsservice_bin_KDEINIT_SRCS org.kde.cervisia5.repository.xml repository.h Repository)

qt_add_dbus_adaptor(cvsservice_bin_KDEINIT_SRCS org.kde.cervisia5.cvsjob.xml cvsjob.h CvsJob)



kf5_add_kdeinit_executable(cvsservice_bin ${cvsservice_bin_KDEINIT_SRCS} NOGUI)
set_target_properties(cvsservice_bin PROPERTIES OUTPUT_NAME cvsservice5)

target_link_libraries(kdeinit_cvsservice_bin Qt::Widgets Qt::DBus KF5::KIOCore KF5::I18n KF5::WidgetsAddons KF5::Su KF5::DBusAddons KF5::ConfigCore)
set_target_properties(kdeinit_cvsservice_bin PROPERTIES OUTPUT_NAME kdeinit5_cvsservice)

install(TARGETS kdeinit_cvsservice_bin ${INSTALL_TARGETS_DEFAULT_ARGS} )

target_link_libraries( cvsservice_bin kdeinit_cvsservice_bin )
install(TARGETS cvsservice_bin  ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### next target ###############

kf5_add_kdeinit_executable(cvsaskpass cvsaskpass.cpp)

target_link_libraries(kdeinit_cvsaskpass Qt::Widgets KF5::WidgetsAddons KF5::CoreAddons KF5::I18n)

install(TARGETS kdeinit_cvsaskpass ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(TARGETS cvsaskpass  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( FILES  org.kde.cvsservice5.desktop  DESTINATION  ${KDE_INSTALL_KSERVICESDIR} )

install( FILES org.kde.cervisia5.cvsjob.xml
               org.kde.cervisia5.cvsservice.xml
               org.kde.cervisia5.repository.xml
               org.kde.cervisia5.cvsloginjob.xml
         DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR} )
