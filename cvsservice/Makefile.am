INCLUDES = $(all_includes)

bin_PROGRAMS = cvsservice
cvsservice_LDFLAGS = $(all_libraries) $(KDE_RPATH)
cvsservice_LDADD = $(LIB_KDEUI)
cvsservice_SOURCES = main.cpp cvsservice.cpp sandbox.cpp cvsjob.cpp \
	cvsservice.skel cvsjob.skel

noinst_HEADERS = cvsservice.h cvsjob.h sandbox.h

METASOURCES = AUTO

service_DATA = cvsservice.desktop
servicedir = $(kde_servicesdir)
