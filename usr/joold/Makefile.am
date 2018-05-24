# Note to myself: documentation tends to call these "PROGRAMS" "targets".
# "joold" is a "target".

bin_PROGRAMS = joold
#joold_SOURCES = nothing needed here anymore. See jool_*_SOURCES.

joold_LDADD = ${LIBNLGENL3_LIBS}
joold_CFLAGS = -Wall -pedantic -O2
joold_CFLAGS += -I${srcdir}/../../include
joold_CFLAGS += ${LIBNLGENL3_CFLAGS} ${JOOL_FLAGS} -DJOOLD
man_MANS = joold.8


# https://stackoverflow.com/questions/30379837
#
# See the corresponding massive comment in ../stateful/Makefile.am for relevant
# information.

joold_stateful_SOURCES = xlat.c
joold_netlink_SOURCES = config.c
joold_src_SOURCES = \
	cJSON.c \
	file.c \
	joold.c \
	log.c \
	modsocket.c \
	netlink2.c \
	netsocket.c \
	str_utils.c

nodist_joold_SOURCES = \
	$(joold_stateful_SOURCES) \
	$(joold_netlink_SOURCES) \
	$(joold_src_SOURCES)
CLEANFILES = \
	$(joold_stateful_SOURCES) \
	$(joold_netlink_SOURCES) \
	$(joold_src_SOURCES)

$(joold_stateful_SOURCES):
	echo "#include \"$(top_builddir)/../common/stateless/$@\"" > $@

$(joold_netlink_SOURCES):
	echo "#include \"$(top_builddir)/../common/netlink/$@\"" > $@

$(joold_src_SOURCES):
	echo "#include \"$(top_builddir)/src/$@\"" > $@
