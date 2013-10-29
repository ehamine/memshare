AM_CPPFLAGS = -I$(top_srcdir)/src -Wall

if MEMCMD
MEMPROGS = memwatch memsend
endif

if TESTS
MEMTST = basetest reply listen tlogtest
endif

bin_PROGRAMS = $(MEMPROGS)
noinst_PROGRAMS = $(MEMTST)
memsend_SOURCES = src/memsend.c
memsend_LDADD = libmemshare.la -lpthread
memwatch_SOURCES = src/memwatch.c
memwatch_LDADD = libmemshare.la -lpthread
basetest_SOURCES = src/test/testmain.c
basetest_LDADD = libmemshare.la -lpthread
reply_SOURCES = src/test/reply.c
reply_LDADD = libmemshare.la -lpthread
listen_SOURCES = src/test/listen.c
listen_LDADD = libmemshare.la -lpthread
tlogtest_SOURCES = src/examples/tlogmain.c
tlogtest_LDADD = libtlog.la -lpthread

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
