#
#                        SvnService Makefile.am
#
#  Copyright (c) 2005 Christian Loose <christian.loose@kdemail.net>
#
#

INCLUDES = $(all_includes)

# Subversion DCOP service
bin_PROGRAMS = svnservice

svnservice_LDFLAGS = $(all_libraries) $(KDE_RPATH)
svnservice_LDADD = $(LIB_KIO)
svnservice_SOURCES = main.cpp svnservice.cpp svnjob.cpp \
	svnservice.skel svnservice.stub svnjob.skel svnjob.stub \
	repository.cpp repository.skel repository.stub

include_HEADERS = svnservice_stub.h svnjob_stub.h repository_stub.h
noinst_HEADERS = svnservice.h svnjob.h repository.h

# Subversion DCOP service stub library
lib_LTLIBRARIES = libsvnservice.la

libsvnservice_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 0:1
libsvnservice_la_LIBADD = $(LIB_KDECORE)
libsvnservice_la_SOURCES = svnservice.stub svnjob.stub repository.stub

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

# install .desktop file
service_DATA = svnservice.desktop
servicedir = $(kde_servicesdir)

# i18n
messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/svnservice.pot
