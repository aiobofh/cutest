Q=@

.SUFFIXES:

all:
	$(Q)$(MAKE) -s -r --no-print-directory $(DEPS) && \
	$(MAKE) -r --no-print-directory check && \
	$(MAKE) -r --no-print-directory apassbc

%.d: %.c
	$(Q)$(CC) -MM -MG $(CFLAGS) $< | grep -v -e '^ \\'>> $@

include ../src/cutest.mk
include ../src/coverage.mk

cutest_run_test:: helpers.c
cutest_prox_test:: helpers.c

clean::
	$(Q)$(RM) *~ *.o *.d cutest_run
