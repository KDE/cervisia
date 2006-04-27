kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )

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

kde4_add_dcop_skels(cvsservice_bin_KDEINIT_SRCS 
   cvsservice.h 
   cvsjob.h 
   repository.h 
   cvsloginjob.h )

kde4_add_dcop_stubs(cvsservice_bin_KDEINIT_SRCS 
   cvsservice.h 
   cvsjob.h 
   repository.h )

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

set(cvsservice_LIB_SRCS  )

kde4_add_dcop_stubs(cvsservice_LIB_SRCS
   cvsservice.h
   cvsjob.h
   repository.h )


kde4_automoc(${cvsservice_LIB_SRCS})

kde4_add_library(cvsservice SHARED ${cvsservice_LIB_SRCS})

target_link_libraries(cvsservice  ${KDE4_KDECORE_LIBS} )

set_target_properties(cvsservice PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} cvsservice )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES  cvsservice.desktop )
install_files( /include FILES  cvsservice_stub.h cvsjob_stub.h repository_stub.h )

kde4_footer()



#original Makefile.am contents follow:

##
##                        CvsService Makefile.am
##
##  Copyright (c) 2002-2003 Christian Loose <christian.loose@hamburg.de>
##
##
#
#INCLUDES = $(all_includes)
#
## cvs DCOP service
#bin_PROGRAMS =
#kdeinit_LTLIBRARIES = cvsservice.la cvsaskpass.la
#
#cvsservice_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
#cvsservice_la_LIBADD = $(LIB_KIO)
#cvsservice_la_SOURCES = main.cpp cvsservice.cpp cvsjob.cpp \
#	cvsservice.skel cvsservice.stub cvsjob.skel cvsjob.stub \
#	repository.cpp repository.skel repository.stub sshagent.cpp \
#	cvsserviceutils.cpp cvsloginjob.cpp cvsloginjob.skel cvsloginjob.stub
#
#cvsaskpass_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
#cvsaskpass_la_LIBADD = $(LIB_KDEUI)
#cvsaskpass_la_SOURCES = cvsaskpass.cpp
#
#include_HEADERS = cvsservice_stub.h cvsjob_stub.h repository_stub.h
#noinst_HEADERS = cvsservice.h cvsjob.h repository.h cvsserviceutils.h \
#	sshagent.h
#
## cvs DCOP service stub library
#lib_LTLIBRARIES = libcvsservice.la
#
#libcvsservice_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 0:1
#libcvsservice_la_LIBADD = $(LIB_KDECORE)
#libcvsservice_la_SOURCES = cvsservice.stub cvsjob.stub repository.stub dummy.cpp
#
#dummy.cpp:
#	echo > dummy.cpp
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
## install .desktop file
#service_DATA = cvsservice.desktop
#servicedir = $(kde_servicesdir)
#
## i18n
#messages:
#	$(XGETTEXT) *.cpp *.h -o $(podir)/cvsservice.pot
#
## API documentation
## Not activated because KDE_INIT_DOXYGEN is missing in
## kdesdk's configure.in.in
##DOXYGEN_REFERENCES = dcop kdecore kdeui
##include ../../admin/Doxyfile.am
