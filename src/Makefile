Q=@

.SUFFIXES:

DEPS=$(subst .c,.d,$(wildcard *.c))

all:
	$(Q)$(MAKE) -s -r --no-print-directory $(DEPS) && \
	$(MAKE) -s -r --no-print-directory check

%.d: %.c
	$(Q)$(CC) -MM -MG $(CFLAGS) $< | grep -v -e '^ \\'>> $@

include ../src/cutest.mk
include ../src/coverage.mk

cutest_run_test:: helpers.c testcase.c
cutest_prox_test:: helpers.c
cutest_mock_test:: helpers.c arg.c mockable.c
cutest_work_test:: helpers.c
mockable_test:: arg.c

clean::
	$(Q)$(RM) *~ *.o *.d cutest_run
