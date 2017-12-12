#include cutest.mk
#
#ifneq ("$(COV)","")
#	include coverage.mk
#endif

Q=@

all: cutest_run cutest_mock cutest_prox cutest_work

STD:=none
HAS_C11:=$(shell $(CC) -std=c11 -o empty empty.c 2>&1 >/dev/null && echo "yes")
ifeq ("$(HAS_C11)","yes")
	STD:=-std=c11
else
	HAS_C11:=no
	HAS_C99:=$(shell $(CC) -std=c99 -o empty empty.c 2>&1 >/dev/null && echo "yes")
	ifeq ("$(HAS_C99)","yes")
		STD:=-std=c99
	else
		HAS_C99:=no
		HAS_C90:=$(shell $(CC) -std=c90 -o empty empty.c 2>&1 >/dev/null && echo "yes")
		ifeq ("$(HAS_C90)","yes")
			STD:=-std=c90
		else
			PEDANTIC:=
			STD:=
		endif
	endif
endif

HAS_LTO:=$(shell $(CC) -o empty empty.c -flto 2>&1 >/dev/null && echo "yes")
ifeq ("$(HAS_LTO)","yes")
	LTO:=-flto
else
	LTO:=
	HAS_LTO:=no
endif

LOCALCFLAGS=-pedantic -Wall -O2 $(STD) $(LTO)

info:
	$(Q)echo "C-standard: $(STD)"
	$(Q)echo "LTO       : $(LTO)"
	$(Q)echo "CLFAGS    : $(LOCALCFLAGS)"

%.o: %.c
	$(Q)$(CC) -c $^ $(LOCALCFLAGS) $(EXTRA_CFLAGS) -o $@

cutest_run: cutest_run.o helpers.o testcase.o
	$(Q)$(CC) $^ $(LOCALCFLAGS) $(EXTRA_CFLAGS) -o $@

cutest_mock: cutest_mock.o helpers.o mockable.o arg.o
	$(Q)$(CC) $^ $(LOCALCFLAGS) $(EXTRA_CFLAGS) -o $@

cutest_prox: cutest_prox.o helpers.o
	$(Q)$(CC) $^ $(LOCALCFLAGS) $(EXTRA_CFLAGS) -o $@

cutest_work: cutest_work.o helpers.o
	$(Q)$(CC) $^ $(LOCALCFLAGS) $(EXTRA_CFLAGS) -o $@

check:
	$(Q)$(MAKE) -s -r --no-print-directory -f cutest.mk check

clean:
	$(Q)$(RM) *~ *.o cutest_run cutset_mock cutest_prox cutest_work empty && \
	$(MAKE) -s -r --no-print-directory -f cutest.mk clean \
