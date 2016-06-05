project(cervisia)

cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
set(QT_MIN_VERSION "5.2.0")

find_package(ECM 1.3.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

add_definitions(-DTRANSLATION_DOMAIN="cervisia")

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMInstallIcons)
include(ECMAddAppIcon)
include(ECMMarkNonGuiExecutable)
include(FeatureSummary)
#include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Widgets DBus)

find_package(KF5 REQUIRED COMPONENTS Parts DocTools Init IconThemes Notifications WidgetsAddons ItemViews Su)

add_subdirectory( doc )
add_subdirectory( cvsservice )
add_subdirectory( pics )

set(_KDECONFDIR_ ${CONFIG_INSTALL_DIR})
set(_KDEHTMLDIR_ ${HTML_INSTALL_DIR})

########### next target ###############

set(libcervisia_SRCS
   annotatedialog.cpp
   diffdialog.cpp
   patchoptiondialog.cpp
   logdialog.cpp
   progressdialog.cpp
   resolvedialog.cpp
   resolvedialog_p.cpp
   annotateview.cpp
   diffview.cpp
   loglist.cpp
   logplainview.cpp
   logtree.cpp
   annotatecontroller.cpp
   loginfo.cpp
   misc.cpp
   qttableview.cpp
   tooltip.cpp
   settingsdialog.cpp
   debug.cpp
   )

qt5_add_dbus_interfaces( libcervisia_SRCS cvsservice/org.kde.cervisia5.cvsservice.xml )
qt5_add_dbus_interfaces( libcervisia_SRCS cvsservice/org.kde.cervisia5.cvsjob.xml )
qt5_add_dbus_interfaces( libcervisia_SRCS cvsservice/org.kde.cervisia5.cvsloginjob.xml )
qt5_add_dbus_interfaces( libcervisia_SRCS cvsservice/org.kde.cervisia5.repository.xml )


ki18n_wrap_ui(libcervisia_SRCS settingsdialog_advanced.ui )

kconfig_add_kcfg_files(libcervisia_SRCS cervisiasettings.kcfgc )


set(cervisia_KDEINIT_SRCS ${libcervisia_SRCS}
   main.cpp
   cervisiashell.cpp )

kf5_add_kdeinit_executable(cervisia ${cervisia_KDEINIT_SRCS})

target_link_libraries(kdeinit_cervisia KF5::Parts KF5::ItemViews)

install(TARGETS kdeinit_cervisia ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(TARGETS cervisia  ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### next target ###############

set(cervisiapart_PART_SRCS ${libcervisia_SRCS}
   updateview.cpp
   protocolview.cpp
   watchdialog.cpp
   changelogdialog.cpp
   historydialog.cpp
   repositorydialog.cpp
   commitdialog.cpp
   checkoutdialog.cpp
   updatedialog.cpp
   tagdialog.cpp
   mergedialog.cpp
   cvsdir.cpp
   repositories.cpp
   cervisiapart.cpp
   addrepositorydialog.cpp
   addremovedialog.cpp
   watchersdialog.cpp
   watchersmodel.cpp
   updateview_items.cpp
   updateview_visitors.cpp
   entry.cpp
   entry_status.cpp
   stringmatcher.cpp
   cvsinitdialog.cpp
   ignorelistbase.cpp
   dirignorelist.cpp
   globalignorelist.cpp
   addignoremenu.cpp
   editwithmenu.cpp
   logmessageedit.cpp
   debug.cpp
)


qt5_add_dbus_adaptor(cervisiapart_PART_SRCS org.kde.cervisia5.protocolview.xml protocolview.h ProtocolView)

add_library(cervisiapart5 MODULE ${cervisiapart_PART_SRCS})

target_link_libraries(cervisiapart5 KF5::Parts KF5::Notifications KF5::ItemViews)

install(TARGETS cervisiapart5  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############


########### install files ###############

install(PROGRAMS org.kde.cervisia5.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install(FILES org.kde.cervisia.appdata.xml DESTINATION  ${DATA_INSTALL_DIR}/appdata )
install(FILES org.kde.cervisiapart5.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES cervisiapart.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install(FILES cervisiaui.rc  DESTINATION  ${KXMLGUI_INSTALL_DIR}/cervisiapart )
install(FILES cervisiashellui.rc cervisia.notifyrc  DESTINATION  ${KXMLGUI_INSTALL_DIR}/cervisia )

ecm_install_icons(ICONS
    16-apps-cervisia.png
    22-apps-cervisia.png
    32-apps-cervisia.png
    48-apps-cervisia.png
    DESTINATION ${ICON_INSTALL_DIR}
    THEME hicolor
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
