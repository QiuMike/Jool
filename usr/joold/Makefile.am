# Note to myself: documentation tends to call these "PROGRAMS" "targets".
# "joold" is a "target".

bin_PROGRAMS = joold
joold_SOURCES = joold.c modsocket.c netsocket.c

joold_LDADD = ${LIBNLGENL3_LIBS}
joold_CFLAGS = -Wall -pedantic -O2
joold_CFLAGS += -I${srcdir}/../../include
joold_CFLAGS += ${LIBNLGENL3_CFLAGS} ${JOOL_FLAGS} -DJOOLD
man_MANS = joold.8


# https://stackoverflow.com/questions/30379837
# I had to collapse all the files into the same (this) directory,
# because portable make is a massive wuss and throws a tantrum
# whenever we want to `$(dir a/b/c.txt)`.

joold_xlat_SOURCES = xlat.c
joold_nl_SOURCES = config.c
joold_usrcommon_SOURCES = \
	cJSON.c \
	file.c \
	log.c \
	netlink2.c \
	str_utils.c

nodist_joold_SOURCES = \
	$(joold_xlat_SOURCES) \
	$(joold_nl_SOURCES) \
	$(joold_usrcommon_SOURCES)
CLEANFILES = \
	$(joold_xlat_SOURCES) \
	$(joold_nl_SOURCES) \
	$(joold_usrcommon_SOURCES)

$(joold_xlat_SOURCES):
	echo "#include \"$(top_builddir)/../common/stateless/$@\"" > $@

$(joold_nl_SOURCES):
	echo "#include \"$(top_builddir)/../common/netlink/$@\"" > $@

$(joold_usrcommon_SOURCES):
	echo "#include \"$(top_builddir)/common/$@\"" > $@
