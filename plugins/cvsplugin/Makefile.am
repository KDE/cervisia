INCLUDES    = -I$(top_srcdir)/cervisia $(all_includes)
METASOURCES = AUTO

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = libcvsplugin.la

libcvsplugin_la_SOURCES = cvsplugin.cpp
#libcvsplugin_la_LIBADD  = $(LIB_KPARTS) $(top_builddir)/cervisia/libcervisia.la
libcvsplugin_la_LIBADD  = $(LIB_KPARTS) $(LIB_KUTILS) $(top_builddir)/cervisia/libcervisia.la \
                          $(top_builddir)/cervisia/cvsservice/libcvsservice.la
libcvsplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

pluginsdir = $(kde_datadir)/cervisiapart/kpartplugins
plugins_DATA = cvsplugin.desktop

# NEEDED!
#messages: rc.cpp
#        $(XGETTEXT) *.cpp *.h -o $(podir)/webarchiver.pot