Q ?=@
.SUFFIXES:

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

CFLAGS+=-Wall -pedantic -O2 $(STD)
LDFLAGS+=-flto

export CFLAGS
export LDFLAGS

all: cutest_prox cutest_mock cutest_run
	$(Q)echo "Running unit-tests" && \
	$(MAKE) -r --no-print-directory -C test

helpers.o: helpers.c helpers.h
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

mockable.o: mockable.c mockable.h arg.h list.h
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

arg.o: arg.c arg.h list.h
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

testcase.o: testcase.c testcase.h list.h
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

#cutest_impl.o: cutest_impl.c cutest.h cutest_lenient.h cutest_simple.h
#	$(Q)$(CC) $(CFLAGS) -c $< -o $@

cutest_prox.o: cutest_prox.c cutest_prox.h helpers.h
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

cutest_mock.o: cutest_mock.c cutest_mock.h arg.h mockable.h list.h helpers.h
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

cutest_run.o: cutest_run.c cutest_run.h testcase.h helpers.h
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

cutest_prox: cutest_prox.o helpers.o
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

cutest_mock: cutest_mock.o helpers.o arg.o mockable.o helpers.o
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

cutest_run: cutest_run.o helpers.o testcase.o
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	$(Q)$(MAKE)  -r --no-print-directory -C test clean
	$(Q)$(RM) -f helpers.o mockable.o arg.o testcase.o cutest_impl.o cutest_prox.o cutest_mock.o cutest_run.o cutest_prox cutest_mock cutest_run
