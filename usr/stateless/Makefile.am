# Note to myself: documentation tends to call these "PROGRAMS" "targets".
# "jool_siit" is a "target".

bin_PROGRAMS = jool_siit
#jool_siit_SOURCES = nothing needed here anymore. See jool_siit_*_SOURCES.

jool_siit_LDADD = ${LIBNLGENL3_LIBS}
jool_siit_CFLAGS = -Wall -O2
jool_siit_CFLAGS += -I${srcdir}/../../include
jool_siit_CFLAGS += ${LIBNLGENL3_CFLAGS} ${JOOL_FLAGS}
man_MANS = jool_siit.8


# https://stackoverflow.com/questions/30379837
# I had to collapse all the files into the same (this) directory,
# because portable make is a massive wuss and throws a tantrum
# whenever we want to `$(dir a/b/c.txt)`.

jool_siit_xlat_SOURCES = xlat.c
jool_siit_nl_SOURCES = config.c
jool_siit_usrcommon_SOURCES = \
	argp_options.c \
	cJSON.c \
	dns.c \
	file.c \
	jool.c \
	netlink2.c \
	nl_buffer.c \
	str_utils.c
jool_siit_target_SOURCES = \
	bib.c \
	eam.c \
	global.c \
	instance.c \
	joold.c \
	json.c \
	pool.c \
	pool4.c \
	pool6.c \
	session.c

nodist_jool_siit_SOURCES = \
	$(jool_siit_xlat_SOURCES) \
	$(jool_siit_nl_SOURCES) \
	$(jool_siit_usrcommon_SOURCES) \
	$(jool_siit_target_SOURCES)
CLEANFILES = \
	$(jool_siit_xlat_SOURCES) \
	$(jool_siit_nl_SOURCES) \
	$(jool_siit_usrcommon_SOURCES) \
	$(jool_siit_target_SOURCES)

$(jool_siit_xlat_SOURCES):
	echo "#include \"$(top_builddir)/../common/stateless/$@\"" > $@

$(jool_siit_nl_SOURCES):
	echo "#include \"$(top_builddir)/../common/netlink/$@\"" > $@

$(jool_siit_usrcommon_SOURCES):
	echo "#include \"$(top_builddir)/common/$@\"" > $@

$(jool_siit_target_SOURCES):
	echo "#include \"$(top_builddir)/common/target/$@\"" > $@
