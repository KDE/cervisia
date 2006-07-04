


########### next target ###############

set(cvsservice_bin_KDEINIT_SRCS 
   main.cpp 
   cvsservice.cpp 
   cvsjob.cpp 
   repository.cpp 
   sshagent.cpp 
   cvsserviceutils.cpp 
   cvsloginjob.cpp) 

kde4_automoc(${cvsservice_bin_KDEINIT_SRCS})


kde4_add_kdeinit_executable( cvsservice_bin ${cvsservice_bin_KDEINIT_SRCS})
set_target_properties(cvsservice_bin PROPERTIES OUTPUT_NAME cvsservice)


target_link_libraries(kdeinit_cvsservice_bin  ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} )
set_target_properties(kdeinit_cvsservice_bin PROPERTIES OUTPUT_NAME kdeinit_cvsservice)

install_targets(${LIB_INSTALL_DIR} kdeinit_cvsservice_bin )

target_link_libraries( cvsservice_bin kdeinit_cvsservice_bin )
install_targets(/bin cvsservice_bin )

########### next target ###############

set(cvsaskpass_KDEINIT_SRCS cvsaskpass.cpp )

kde4_automoc(${cvsaskpass_KDEINIT_SRCS})

kde4_add_kdeinit_executable( cvsaskpass ${cvsaskpass_KDEINIT_SRCS})

target_link_libraries(kdeinit_cvsaskpass  ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} kdesu )

install_targets(${LIB_INSTALL_DIR} kdeinit_cvsaskpass )

target_link_libraries( cvsaskpass kdeinit_cvsaskpass )
install_targets(/bin cvsaskpass )

########### next target ###############

qt4_add_dbus_adaptor(cvsservice_bin_KDEINIT_SRCS org.kde.cervisia.cvsservice.xml cvsservice.h CvsService)
qt4_add_dbus_adaptor(cvsservice_bin_KDEINIT_SRCS org.kde.cervisia.cvsjob.xml cvsjob.h CvsJob)
qt4_add_dbus_adaptor(cvsservice_bin_KDEINIT_SRCS org.kde.cervisia.repository.xml repository.h Repository)

########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES  cvsservice.desktop )
install_files( /include FILES  cvsservice_stub.h cvsjob_stub.h repository_stub.h )
