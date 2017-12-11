LOCALCFLAGS=-pedantic -Wall -std=c99 -O2

ifneq ($(findstring clang,$(CC)),clang)
	LOCALCFLAGS+=-flto
endif

all:
	$(MAKE) -s -r --no-print-directory cutest_run && \
	$(MAKE) -s -r --no-print-directory cutest_mock && \
	$(MAKE) -s -r --no-print-directory cutest_prox && \
	$(MAKE) -s -r --no-print-directory cutest_work

include cutest.mk
include coverage.mk

%.o: %.c
	$(CC) -c $^ $(LOCALCFLAGS) $(EXTRA_CFLAGS) -o $@

cutest_run: cutest_run.o helpers.o testcase.o
	$(CC) $^ $(LOCALCFLAGS) $(EXTRA_CFLAGS) -o $@

cutest_mock: cutest_mock.o helpers.o mockable.o arg.o
	$(CC) $^ $(LOCALCFLAGS) $(EXTRA_CFLAGS) -o $@

cutest_prox: cutest_prox.o helpers.o
	$(CC) $^ $(LOCALCFLAGS) $(EXTRA_CFLAGS) -o $@

cutest_work: cutest_work.o helpers.o
	$(CC) $^ $(LOCALCFLAGS) $(EXTRA_CFLAGS) -o $@

clean::
	$(Q)$(RM) *~ *.o cutest_run cutset_mock cutest_prox cutest_work

#
# Dependencies described for test cases
#
cutest_run_test: helpers.c testcase.c

cutest_mock_test: helpers.c mockable.c arg.c

cutest_prox_test: helpers.c

cutest_work_test: helpers.c

mockable_test: arg.c list.c
