#
#                        CvsService Makefile.am
#
#  Copyright (c) 2002-2003 Christian Loose <christian.loose@hamburg.de>
#
#

INCLUDES = $(all_includes)

# cvs DCOP service
bin_PROGRAMS = cvsservice

cvsservice_LDFLAGS = $(all_libraries) $(KDE_RPATH)
cvsservice_LDADD = $(LIB_KDEUI) $(LIB_KIO)
cvsservice_SOURCES = main.cpp cvsservice.cpp cvsjob.cpp \
	cvsservice.skel cvsservice.stub cvsjob.skel cvsjob.stub \
	repository.cpp

include_HEADERS = cvsservice_stub.h cvsjob_stub.h
noinst_HEADERS = cvsservice.h cvsjob.h repository.h

# cvs DCOP service stub library
lib_LTLIBRARIES = libcvsservice.la

libcvsservice_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 0:1
libcvsservice_la_LIBADD = $(LIB_KDECORE)
libcvsservice_la_SOURCES = cvsservice_stub.cpp cvsjob_stub.cpp

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
