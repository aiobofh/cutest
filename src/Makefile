include cutest.mk

ifneq ("$(COV)","")
	include coverage.mk
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

clean::
	$(Q)$(RM) *~ *.o cutest_run cutset_mock cutest_prox cutest_work

clean_all: clean clean_cutest clean_cproto

#
# Dependencies described for test cases
#
cutest_run_test: helpers.c testcase.c

cutest_mock_test: helpers.c mockable.c arg.c

cutest_prox_test: helpers.c

cutest_work_test: helpers.c

mockable_test: arg.c list.c
