#
#                        CvsService Makefile.am
#
#  Copyright (c) 2002-2003 Christian Loose <christian.loose@hamburg.de>
#
#

INCLUDES = $(all_includes)

# cvs DCOP service
bin_PROGRAMS =
kdeinit_LTLIBRARIES = cvsservice.la cvsaskpass.la

cvsservice_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
cvsservice_la_LIBADD = $(LIB_KIO)
cvsservice_la_SOURCES = main.cpp cvsservice.cpp cvsjob.cpp \
	cvsservice.skel cvsservice.stub cvsjob.skel cvsjob.stub \
	repository.cpp repository.skel repository.stub sshagent.cpp \
	cvsserviceutils.cpp cvsloginjob.cpp cvsloginjob.skel cvsloginjob.stub

cvsaskpass_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
cvsaskpass_la_LIBADD = $(LIB_KDEUI)
cvsaskpass_la_SOURCES = cvsaskpass.cpp

include_HEADERS = cvsservice_stub.h cvsjob_stub.h repository_stub.h
noinst_HEADERS = cvsservice.h cvsjob.h repository.h cvsserviceutils.h \
	sshagent.h

# cvs DCOP service stub library
lib_LTLIBRARIES = libcvsservice.la

libcvsservice_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 0:1
libcvsservice_la_LIBADD = $(LIB_KDECORE)
libcvsservice_la_SOURCES = cvsservice.stub cvsjob.stub repository.stub dummy.cpp

dummy.cpp:
	echo > dummy.cpp

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

# install .desktop file
service_DATA = cvsservice.desktop
servicedir = $(kde_servicesdir)

# i18n
messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/cvsservice.pot

# API documentation
# Not activated because KDE_INIT_DOXYGEN is missing in
# kdesdk's configure.in.in
#DOXYGEN_REFERENCES = dcop kdecore kdeui
#include ../../admin/Doxyfile.am
