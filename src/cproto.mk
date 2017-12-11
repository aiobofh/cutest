#
# cproto.mk
#
# This is a naive build system for producing a cproto exeutable that is
# available to CUTest, since CUTest heavily relys on cproto perfomance.
#
# The cproto distribution it self is quite complete, however since the goal
# with CUTest is to have as few dependencies to other tools as possible this
# makefile tries to circumvent the use of autoconf things to make it easier
# (and faster) to compile and use cutest on low-end/legacy machines.
#
# This makefile will try to download the cproto-4.7m version from the autor's
# website/ftpsite, and compile it with as few dependencies as possible.
#
# About cproto:
#
# cproto - generate C function prototypes and convert function definitions
#
# Authors:
#
# Chin Huang
# cthuang@vex.net
# cthuang@interlog.com
#
# Thomas Dickey
# dickey@invisible-island.net
# modifications to support lint library, type-copying, and port to VAX/VMS.
#
# I (the author of CUTest) give you many thanks and hope that you are OK with
# CUTest making use of your magnificent tool!!!
#
# My notes about cproto is that it is an awesome tool, and maybe I will try to
# learn from it and integrate some of the ideas directly into CUTest some day,
# however as of now it will have to rely on the execution of the "cproto"
# binary to get a nice and easy parsing of function prototypes.
#

CPROTO_VER=4.7m

CPROTO_PATH=$(CUTEST_PATH)/cproto-$(CPROTO_VER)

CPROTO=$(CPROTO_PATH)/cproto
INSTALLED_CPROTO:=$(shell which cproto)
ifneq ("$(INSTALLED_CPROTO)","")
	CPROTO=$(INSTALLED_CPROTO)
endif

BISON=$(error bison - GNU Project parser generator not found. Can not build cproto-4.7m. You will have to install cproto onto your system manually. For more infomation go to http://invisible-island.net/cproto/cproto.html)
ifneq ("$(wildcard /usr/bin/bison)","")
	BISON=/usr/bin/bison
else
	ifneq ("$(wildcard /bin/bison)","")
		BISON=GG:bin/bison
	endif
endif

FLEX=$(error flex - the fast lexical analyser generator not found. Can not build cproto-4.7m. You will have to install cproto onto your system manually. For more infomation go to http://invisible-island.net/cproto/cproto.html)
ifneq ("$(wildcard /usr/bin/flex)","")
	FLEX=/usr/bin/flex
else
	ifneq ("$(wildcard /bin/flex)","")
		FLEX=/bin/flex
	endif
endif

WGET=$(error Wget - The non-interactive network downloader not found. Can not build cproto-4.7m. You will have to download it manually and put the cproto-4.7m.tgz in $(CUTEST_PATH). For more infomation go to http://invisible-island.net/cproto/cproto.html)
ifneq ("$(wildcard /usr/bin/wget)","")
	WGET=/usr/bin/wget
else
	ifneq ("$(wildcard /bin/wget)","")
		WGET=/bin/wget
	endif
endif

TAR=$(error tar - The GNU version of the tar archiving utility not found. Can not build cproto-4.7m. You will have to install cproto onto your system manually. For more infomation go to http://invisible-island.net/cproto/cproto.html)
ifneq ("$(wildcard /usr/bin/tar)","")
	TAR=/usr/bin/tar
else
	ifneq ("$(wildcard /bin/tar)","")
		TAR=/bin/tar
	endif
endif

RMDIR=$(RM) -r
CPROTO_FLAGS=-D"CPP_DOES_ONLY_C_FILES=1" -D"CPP=\"gcc -E\"" -D"OPT_LINTLIBRARY=1" -D"MAX_INC_DEPTH=999999"

$(CUTEST_PATH)/cproto-4.7m.tgz:
	$(info Using $(WGET) to download cproto-4.7m from authors site)
	$(Q)$(WGET) -q ftp://ftp.invisible-island.net/cproto/cproto-4.7m.tgz

$(CPROTO_PATH): $(CUTEST_PATH)/cproto-4.7m.tgz
	$(Q)$(TAR) xzf $<

$(CPROTO_PATH)/lex.l: $(CPROTO_PATH)

$(CPROTO_PATH)/cproto.c: $(CPROTO_PATH)

$(CPROTO_PATH)/dump.c: $(CPROTO_PATH)

$(CPROTO_PATH)/lintlibs.c: $(CPROTO_PATH)

$(CPROTO_PATH)/semantics.c: $(CPROTO_PATH)

$(CPROTO_PATH)/strkey.c: $(CPROTO_PATH)

$(CPROTO_PATH)/symbol.c: $(CPROTO_PATH)

$(CPROTO_PATH)/trace.c: $(CPROTO_PATH)

$(CPROTO_PATH)/yyerror.c: $(CPROTO_PATH)

$(CPROTO_PATH)/lex.yy.c: $(CPROTO_PATH)/lex.l $(FLEX)
	$(info Using $(FLEX) to build parts of cproto-4.7m)
	$(Q)cd cproto-4.7m && $(FLEX) $<

$(CPROTO_PATH)/grammar.tab.c: $(CPROTO_PATH)/grammar.y $(CPROTO_PATH)/lex.yy.c $(BISON)
	$(info Using $(BISON) to build parts of cproto-4.7m)
	$(Q)cd cproto-4.7m && $(BISON) $(notdir $<)

$(CPROTO_PATH)/cproto: $(CPROTO_PATH)/cproto.c $(CPROTO_PATH)/dump.c $(CPROTO_PATH)/lintlibs.c $(CPROTO_PATH)/semantic.c $(CPROTO_PATH)/strkey.c $(CPROTO_PATH)/symbol.c $(CPROTO_PATH)/trace.c $(CPROTO_PATH)/yyerror.c $(CPROTO_PATH)/grammar.tab.c
	$(Q)$(CC) -o $@ $^ -Icproto-4.7m/ $(CPROTO_FLAGS)

clean_cproto:
	$(Q)$(RMDIR) $(CPROTO_PATH)
	$(Q)$(RM) cproto-4.7m.tgz
