#
#                        CvsPlugin Makefile.am
#
#  Copyright (c) 2003 Christian Loose <christian.loose@hamburg.de>
#
#

AM_CPPFLAGS = -I.. $(all_includes)

# cvs plugin
kde_module_LTLIBRARIES = cvsinterfaceplugin.la

cvsinterfaceplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
cvsinterfaceplugin_la_LIBADD = $(LIB_KDECORE) $(LIB_KIO)
cvsinterfaceplugin_la_SOURCES = cvsrepository.cpp cvspluginfactory.cpp

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO
