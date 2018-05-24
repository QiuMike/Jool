# Note to myself: documentation tends to call these "PROGRAMS" "targets".
# "jool" is a "target".

bin_PROGRAMS = jool
#jool_SOURCES = nothing needed here anymore. See jool_*_SOURCES.

jool_LDADD = ${LIBNLGENL3_LIBS}
jool_CFLAGS = -Wall -pedantic -O2
jool_CFLAGS += -I${srcdir}/../../include
jool_CFLAGS += ${LIBNLGENL3_CFLAGS} ${JOOL_FLAGS}
man_MANS = jool.8


# https://stackoverflow.com/questions/30379837
#
# We used to suffer from the same problem. And the accepted solution would have
# been fine, except we have several common directories; not just one.
#
# (For the purposes of this discussion, keep in mind that when I say "proxy
# file", I'm referring to those .c files that only #include some other .c file.)
#
# I would have liked to do something like this:
#
#	jool_common_SOURCES = common/stateful/xlat.c common/netlink/config.c usr/common/argp_options.c <etc etc>
#	$(jool_stateful_SOURCES):
#		echo "#include \"$(top_builddir)/../$@\"" > $@
#
# But it doesn't work because `echo` cannot send its output to a file whose
# parent directories haven't been `mkdir`d.
# And I cannot `mkdir` because we do not have access to the GNU Make file name
# functions (Debian packaging wants us to use stock make for the sake of
# portability), so I can't find a way to obtain the parent directories of the
# `jool_common_SOURCES` files. (Which is really sad because extracting
# `common/stateful` out of `common/stateful/xlat.c` should be pancakes.)
#
# The least intrusive solution that I found with my limited make skills was to
# separate `jool_stateful_SOURCES` into several `_SOURCE` variables, one for
# each directory that we need to extract common source from. In this way, the
# `_SOURCE` variable only contains the name of the files, and the directory they
# belong to is encoded in their respective build rule.
#
# Not including the directories in the `_SOURCE` variables, in turn, forced me
# to place all the proxy files in the same directory (because of `> $@`).
#
# That should have been all, but there's still one problem: One consequence of
# our inability to rename proxy files or place them in special directories is
# that it's impossible for .gitignore to tell the difference between a proxy
# file and a normal .c file. (Proxy files should not be repositoried.) I don't
# want to enumerate proxy files in .gitignore one by one, so I came up with the
# idea of moving all normal .c files into `../src/`, and reserve `./`,
# `../stateless/` and `../joold/` for proxy files.
#
# So here's a rule: DO NOT add any real .c files to this directory; it's
# reserved for proxy .c files.

jool_stateful_SOURCES = xlat.c
jool_netlink_SOURCES = config.c
jool_src_SOURCES = \
	argp_options.c \
	cJSON.c \
	dns.c \
	file.c \
	jool.c \
	netlink2.c \
	nl_buffer.c \
	str_utils.c
jool_target_SOURCES = \
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

nodist_jool_SOURCES = \
	$(jool_stateful_SOURCES) \
	$(jool_netlink_SOURCES) \
	$(jool_src_SOURCES) \
	$(jool_target_SOURCES)
CLEANFILES = \
	$(jool_stateful_SOURCES) \
	$(jool_netlink_SOURCES) \
	$(jool_src_SOURCES) \
	$(jool_target_SOURCES)

$(jool_stateful_SOURCES):
	echo "#include \"$(top_builddir)/../common/stateful/$@\"" > $@

$(jool_netlink_SOURCES):
	echo "#include \"$(top_builddir)/../common/netlink/$@\"" > $@

$(jool_src_SOURCES):
	echo "#include \"$(top_builddir)/src/$@\"" > $@

$(jool_target_SOURCES):
	echo "#include \"$(top_builddir)/src/target/$@\"" > $@
