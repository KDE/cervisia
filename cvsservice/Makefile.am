INCLUDES = $(all_includes)

bin_PROGRAMS = cvsservice
cvsservice_LDFLAGS = $(all_libraries) $(KDE_RPATH)
cvsservice_LDADD = $(LIB_KDEUI) $(LIB_KIO)
cvsservice_SOURCES = main.cpp cvsservice.cpp cvsjob.cpp \
	cvsservice.skel cvsjob.skel repository.cpp

noinst_HEADERS = cvsservice.h cvsjob.h repository.h

METASOURCES = AUTO

service_DATA = cvsservice.desktop
servicedir = $(kde_servicesdir)
