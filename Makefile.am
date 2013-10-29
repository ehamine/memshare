AM_CPPFLAGS = -I$(top_srcdir)/src -Wall -pthread
AM_LDFLAGS = -pthread

if MEMCMD
bin_PROGRAMS = memsend memwatch
memsend_SOURCES = src/memsend.c
memsend_LDADD = libmemshare.la
memwatch_SOURCES = src/memwatch.c
memwatch_LDADD = libmemshare.la
endif

#TESTS = basetest reply listen
#check_PROGRAMS = basetest reply listen
basetest_SOURCES = src/test/testmain.c
basetest_LDADD = libmemshare.la
reply_SOURCES = src/test/reply.c
reply_LDADD = libmemshare.la
listen_SOURCES = src/test/listen.c
listen_LDADD = libmemshare.la
tlogtest_SOURCES = src/examples/tlogmain.c
tlogtest_LDADD = libtlog.la

lib_LTLIBRARIES = libmemshare.la
libmemshare_la_SOURCES = src/memshare.c \
			src/memshare.h \
			src/queue.c \
			src/queue.h \
			src/memshare_api.h
include_HEADERS = src/memshare_api.h

if TLOG
libtlog_la_SOURCES = src/examples/tlog.c \
			src/examples/tlog_api.h
libtlog_la_LIBADD = libmemshare.la
lib_LTLIBRARIES += libtlog.la
include_HEADERS += src/examples/tlog_api.h
endif
