bin_PROGRAMS = hecwfs
hecwfs_SOURCES = hecwfs.c nchelper.c ogrhelper.c printError.c ww3_dayofmonth.c \
	ww3_dayofmonth.h

# set the include path found by configure
INCLUDES = $(all_includes) -I/usr/local/include/

# the library search path.
hecwfs_LDFLAGS = $(all_libraries) 
hecwfs_LDADD = -lnetcdf -lgdal
noinst_HEADERS = nchelper.h ogrhelper.h
