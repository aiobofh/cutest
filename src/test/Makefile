#
# Makefile to run the unit-tests for cutest itself
#
# This file may serve as an example on how to make your own hard-coded-style
# makefiles. It is perfectly possible to automate a lot of things with GNU Make
# and its built-in functions, since there is magic naming conventions in cutest.
#
# However. This example will provide a more insightful knowledge on how to
# construct the build system to run your tests as fast as possible.
#

Q ?=@

HAS_VISIBILITY_HIDDEN:=$(shell $(CC) -o ../empty ../empty.c -fvisibility=hidden 2>&1 >/dev/null && echo "yes")
ifeq ("$(HAS_VISIBILITY_HIDDEN)","yes")
	VISIBILITY_HIDDEN:=-fvisibility=hidden
else
	VISIBILITY_HIDDEN:=
endif

STD:=none
HAS_C11:=$(shell $(CC) -std=c11 -o ../empty ../empty.c 2>&1 >/dev/null && echo "yes")
ifeq ("$(HAS_C11)","yes")
	STD:=-std=c11
else
	HAS_C11:=no
	HAS_C99:=$(shell $(CC) -std=c99 -o ../empty ../empty.c 2>&1 >/dev/null && echo "yes")
	ifeq ("$(HAS_C99)","yes")
		STD:=-std=c99
	else
		HAS_C99:=no
		HAS_C90:=$(shell $(CC) -std=c90 -o ../empty ../empty.c 2>&1 >/dev/null && echo "yes")
		ifeq ("$(HAS_C90)","yes")
			STD:=-std=c90
		else
			PEDANTIC:=
			STD:=
		endif
	endif
endif

LDFLAGS:=-flto
CFLAGS:=-g -Wall -pedantic $(STD)
CPROTO:=$(shell which cproto)
#VISIBILITY_HIDDEN:=-fvisibility=hidden

.SUFFIXES:

.PHONY: all
all: check

.PHONY: info
info:
	@echo "CFLAGS: $(CFLAGS)"

.PRECIPUS: .cutest/timestamp
.cutest/timestamp:
	$(Q)mkdir .cutest && touch $@

.cutest/cutest_impl.o: ../cutest_impl.c
	$(Q)$(CC) -o $@ -c $< $(CFLAGS) -I../

#
# Cutest friendly design-under-test-objects
#
.PRECIOUS: .cutest/arg_mockables.o
.cutest/arg_mockables.o: ../arg.c .cutest/timestamp
	$(Q)$(CC) -o $@ -c $< -I../ $(VISIBILITY_HIDDEN) -fno-inline -g -D"inline="

.PRECIOUS: .cutest/helpers_mockables.o
.cutest/helpers_mockables.o: ../helpers.c .cutest/timestamp
	$(Q)$(CC) -o $@ -c $< -I../ $(VISIBILITY_HIDDEN) -fno-inline -g -D"inline="

.PRECIOUS: .cutest/list_mockables.o
.cutest/list_mockables.o: ../list.c .cutest/timestamp
	$(Q)$(CC) -o $@ -c $< -I../ $(VISIBILITY_HIDDEN) -fno-inline -g -D"inline="

.PRECIOUS: .cutest/mockable_mockables.o
.cutest/mockable_mockables.o: ../mockable.c .cutest/timestamp
	$(Q)$(CC) -o $@ -c $< -I../ $(VISIBILITY_HIDDEN) -fno-inline -g -D"inline="

.PRECIOUS: .cutest/testcase_mockables.o
.cutest/testcase_mockables.o: ../testcase.c .cutest/timestamp
	$(Q)$(CC) -o $@ -c $< -I../ $(VISIBILITY_HIDDEN) -fno-inline -g -D"inline="

.PRECIOUS: .cutest/cutest_mock_mockables.o
.cutest/cutest_mock_mockables.o: ../cutest_mock.c .cutest/timestamp
	$(Q)$(CC) -o $@ -c $< -I../ $(VISIBILITY_HIDDEN) -fno-inline -g -D"inline="

.PRECIOUS: .cutest/cutest_prox_mockables.o
.cutest/cutest_prox_mockables.o: ../cutest_prox.c .cutest/timestamp
	$(Q)$(CC) -o $@ -c $< -I../ $(VISIBILITY_HIDDEN) -fno-inline -g -D"inline="

.PRECIOUS: .cutest/cutest_run_mockables.o
.cutest/cutest_run_mockables.o: ../cutest_run.c .cutest/timestamp
	$(Q)$(CC) -o $@ -c $< -I../ $(VISIBILITY_HIDDEN) -fno-inline -g -D"inline="

#
# Mockbale function symbol lists
#
.PRECIOUS: .cutest/arg_mockables.lst
.cutest/arg_mockables.lst: .cutest/arg_mockables.o  .cutest/timestamp
	$(Q)nm $< | sed 's/.* //g' | grep -v '__stack_' | sort -u > $@ && grep 'gcc2_compiled.' >/dev/null $@ && sed -i 's/^_//g' $@ || true

.PRECIOUS: .cutest/helpers_mockables.lst
.cutest/helpers_mockables.lst: .cutest/helpers_mockables.o  .cutest/timestamp
	$(Q)nm $< | sed 's/.* //g' | grep -v '__stack_' | sort -u > $@ && grep 'gcc2_compiled.' >/dev/null $@ && sed -i 's/^_//g' $@ || true

.PRECIOUS: .cutest/list_mockables.lst
.cutest/list_mockables.lst: .cutest/list_mockables.o  .cutest/timestamp
	$(Q)nm $< | sed 's/.* //g' | grep -v '__stack_' | sort -u > $@ && grep 'gcc2_compiled.' >/dev/null $@ && sed -i 's/^_//g' $@ || true

.PRECIOUS: .cutest/mockable_mockables.lst
.cutest/mockable_mockables.lst: .cutest/mockable_mockables.o  .cutest/timestamp
	$(Q)nm $< | sed 's/.* //g' | grep -v '__stack_' | sort -u > $@ && grep 'gcc2_compiled.' >/dev/null $@ && sed -i 's/^_//g' $@ || true

.PRECIOUS: .cutest/testcase_mockables.lst
.cutest/testcase_mockables.lst: .cutest/testcase_mockables.o  .cutest/timestamp
	$(Q)nm $< | sed 's/.* //g' | grep -v '__stack_' | sort -u > $@ && grep 'gcc2_compiled.' >/dev/null $@ && sed -i 's/^_//g' $@ || true

.PRECIOUS: .cutest/cutest_mock_mockables.lst
.cutest/cutest_mock_mockables.lst: .cutest/cutest_mock_mockables.o  .cutest/timestamp
	$(Q)nm $< | sed 's/.* //g' | grep -v '__stack_' | sort -u > $@ && grep 'gcc2_compiled.' >/dev/null $@ && sed -i 's/^_//g' $@ || true

.PRECIOUS: .cutest/cutest_prox_mockables.lst
.cutest/cutest_prox_mockables.lst: .cutest/cutest_prox_mockables.o  .cutest/timestamp
	$(Q)nm $< | sed 's/.* //g' | grep -v '__stack_' | sort -u > $@ && grep 'gcc2_compiled.' >/dev/null $@ && sed -i 's/^_//g' $@ || true

.PRECIOUS: .cutest/cutest_run_mockables.lst
.cutest/cutest_run_mockables.lst: .cutest/cutest_run_mockables.o  .cutest/timestamp
	$(Q)nm $< | sed 's/.* //g' | grep -v '__stack_' | sort -u > $@ && grep 'gcc2_compiled.' >/dev/null $@ && sed -i 's/^_//g' $@ || true

#
# Assembler versions of the design-under-test-files
#
.PRECIOUS: .cutest/arg_mockables.s
.cutest/arg_mockables.s: ../arg.c .cutest/timestamp
	$(Q)$(CC) -c $< -o $@ -S -fverbose-asm $(VISIBILITY_HIDDEN) -fno-inline -g -O0 $(CFLAGS) -D"static=" -D"inline=" -D"main=MAIN"

.PRECIOUS: .cutest/helpers_mockables.s
.cutest/helpers_mockables.s: ../helpers.c .cutest/timestamp
	$(Q)$(CC) -c $< -o $@ -S -fverbose-asm $(VISIBILITY_HIDDEN) -fno-inline -g -O0 $(CFLAGS) -D"static=" -D"inline=" -D"main=MAIN"

.PRECIOUS: .cutest/list_mockables.s
.cutest/list_mockables.s: ../list.c .cutest/timestamp
	$(Q)$(CC) -c $< -o $@ -S -fverbose-asm $(VISIBILITY_HIDDEN) -fno-inline -g -O0 $(CFLAGS) -D"static=" -D"inline=" -D"main=MAIN"

.PRECIOUS: .cutest/mockable_mockables.s
.cutest/mockable_mockables.s: ../mockable.c .cutest/timestamp
	$(Q)$(CC) -c $< -o $@ -S -fverbose-asm $(VISIBILITY_HIDDEN) -fno-inline -g -O0 $(CFLAGS) -D"static=" -D"inline=" -D"main=MAIN"

.PRECIOUS: .cutest/testcase_mockables.s
.cutest/testcase_mockables.s: ../testcase.c .cutest/timestamp
	$(Q)$(CC) -c $< -o $@ -S -fverbose-asm $(VISIBILITY_HIDDEN) -fno-inline -g -O0 $(CFLAGS) -D"static=" -D"inline=" -D"main=MAIN"

.PRECIOUS: .cutest/cutest_mock_mockables.s
.cutest/cutest_mock_mockables.s: ../cutest_mock.c .cutest/timestamp
	$(Q)$(CC) -c $< -o $@ -S -fverbose-asm $(VISIBILITY_HIDDEN) -fno-inline -g -O0 $(CFLAGS) -D"static=" -D"inline=" -D"main=MAIN"

.PRECIOUS: .cutest/cutest_prox_mockables.s
.cutest/cutest_prox_mockables.s: ../cutest_prox.c .cutest/timestamp
	$(Q)$(CC) -c $< -o $@ -S -fverbose-asm $(VISIBILITY_HIDDEN) -fno-inline -g -O0 $(CFLAGS) -D"static=" -D"inline=" -D"main=MAIN"

.PRECIOUS: .cutest/cutest_run_mockables.s
.cutest/cutest_run_mockables.s: ../cutest_run.c .cutest/timestamp
	$(Q)$(CC) -c $< -o $@ -S -fverbose-asm $(VISIBILITY_HIDDEN) -fno-inline -g -O0 $(CFLAGS) -D"static=" -D"inline=" -D"main=MAIN"

#
# Mockable function lists
#
.PRECIOUS: .cutest/arg_proxified.s
.cutest/arg_proxified.s: .cutest/arg_mockables.s .cutest/arg_mockables.lst
	$(Q)../cutest_prox .cutest/arg_mockables.s .cutest/arg_mockables.lst > $@

.PRECIOUS: .cutest/helpers_proxified.s
.cutest/helpers_proxified.s: .cutest/helpers_mockables.s .cutest/helpers_mockables.lst
	$(Q)../cutest_prox .cutest/helpers_mockables.s .cutest/helpers_mockables.lst > $@

.PRECIOUS: .cutest/list_proxified.s
.cutest/list_proxified.s: .cutest/list_mockables.s .cutest/list_mockables.lst
	$(Q)../cutest_prox .cutest/list_mockables.s .cutest/list_mockables.lst > $@

.PRECIOUS: .cutest/mockable_proxified.s
.cutest/mockable_proxified.s: .cutest/mockable_mockables.s .cutest/mockable_mockables.lst
	$(Q)../cutest_prox .cutest/mockable_mockables.s .cutest/mockable_mockables.lst > $@

.PRECIOUS: .cutest/testcase_proxified.s
.cutest/testcase_proxified.s: .cutest/testcase_mockables.s .cutest/testcase_mockables.lst
	$(Q)../cutest_prox .cutest/testcase_mockables.s .cutest/testcase_mockables.lst > $@

.PRECIOUS: .cutest/cutest_mock_proxified.s
.cutest/cutest_mock_proxified.s: .cutest/cutest_mock_mockables.s .cutest/cutest_mock_mockables.lst
	$(Q)../cutest_prox .cutest/cutest_mock_mockables.s .cutest/cutest_mock_mockables.lst > $@

.PRECIOUS: .cutest/cutest_prox_proxified.s
.cutest/cutest_prox_proxified.s: .cutest/cutest_prox_mockables.s .cutest/cutest_prox_mockables.lst
	$(Q)../cutest_prox .cutest/cutest_prox_mockables.s .cutest/cutest_prox_mockables.lst > $@

.PRECIOUS: .cutest/cutest_run_proxified.s
.cutest/cutest_run_proxified.s: .cutest/cutest_run_mockables.s .cutest/cutest_run_mockables.lst
	$(Q)../cutest_prox .cutest/cutest_run_mockables.s .cutest/cutest_run_mockables.lst > $@

#
# Auto-mocked header files
#
.PRECIOUS: .cutest/arg_mocks.h
.cutest/arg_mocks.h: ../arg.c .cutest/arg_mockables.lst
	$(Q)../cutest_mock $(CPROTO) ../arg.c .cutest/arg_mockables.lst -I.. > $@

.PRECIOUS: .cutest/helpers_mocks.h
.cutest/helpers_mocks.h: ../helpers.c .cutest/helpers_mockables.lst
	$(Q)../cutest_mock $(CPROTO) ../helpers.c .cutest/helpers_mockables.lst -I.. > $@

.PRECIOUS: .cutest/list_mocks.h
.cutest/list_mocks.h: ../list.c .cutest/list_mockables.lst
	$(Q)../cutest_mock $(CPROTO) ../list.c .cutest/list_mockables.lst -I.. > $@

.PRECIOUS: .cutest/mockable_mocks.h
.cutest/mockable_mocks.h: ../mockable.c .cutest/mockable_mockables.lst
	$(Q)../cutest_mock $(CPROTO) ../mockable.c .cutest/mockable_mockables.lst -I.. > $@

.PRECIOUS: .cutest/testcase_mocks.h
.cutest/testcase_mocks.h: ../testcase.c .cutest/testcase_mockables.lst
	$(Q)../cutest_mock $(CPROTO) ../testcase.c .cutest/testcase_mockables.lst -I.. > $@

.PRECIOUS: .cutest/cutest_mock_mocks.h
.cutest/cutest_mock_mocks.h: ../cutest_mock.c .cutest/cutest_mock_mockables.lst
	$(Q)../cutest_mock $(CPROTO) ../cutest_mock.c .cutest/cutest_mock_mockables.lst -I.. > $@

.PRECIOUS: .cutest/cutest_prox_mocks.h
.cutest/cutest_prox_mocks.h: ../cutest_prox.c .cutest/cutest_prox_mockables.lst
	$(Q)../cutest_mock $(CPROTO) ../cutest_prox.c .cutest/cutest_prox_mockables.lst -I.. > $@

.PRECIOUS: .cutest/cutest_run_mocks.h
.cutest/cutest_run_mocks.h: ../cutest_run.c .cutest/cutest_run_mockables.lst
	$(Q)../cutest_mock $(CPROTO) ../cutest_run.c .cutest/cutest_run_mockables.lst -I.. > $@

#
# Test-runner programs
#
.PRECIOUS: .cutest/arg_test_run.c
.cutest/arg_test_run.c: arg_test.c .cutest/arg_mocks.h
	$(Q)../cutest_run arg_test.c arg_mocks.h > $@

.PRECIOUS: .cutest/helpers_test_run.c
.cutest/helpers_test_run.c: helpers_test.c .cutest/helpers_mocks.h
	$(Q)../cutest_run helpers_test.c helpers_mocks.h > $@

.PRECIOUS: .cutest/list_test_run.c
.cutest/list_test_run.c: list_test.c .cutest/list_mocks.h
	$(Q)../cutest_run list_test.c list_mocks.h > $@

.PRECIOUS: .cutest/mockable_test_run.c
.cutest/mockable_test_run.c: mockable_test.c .cutest/mockable_mocks.h
	$(Q)../cutest_run mockable_test.c mockable_mocks.h > $@

.PRECIOUS: .cutest/testcase_test_run.c
.cutest/testcase_test_run.c: testcase_test.c .cutest/testcase_mocks.h
	$(Q)../cutest_run testcase_test.c testcase_mocks.h > $@

.PRECIOUS: .cutest/cutest_mock_test_run.c
.cutest/cutest_mock_test_run.c: cutest_mock_test.c .cutest/cutest_mock_mocks.h
	$(Q)../cutest_run cutest_mock_test.c cutest_mock_mocks.h > $@

.PRECIOUS: .cutest/cutest_prox_test_run.c
.cutest/cutest_prox_test_run.c: cutest_prox_test.c .cutest/cutest_prox_mocks.h
	$(Q)../cutest_run cutest_prox_test.c cutest_prox_mocks.h > $@

.PRECIOUS: .cutest/cutest_run_test_run.c
.cutest/cutest_run_test_run.c: cutest_run_test.c .cutest/cutest_run_mocks.h
	$(Q)../cutest_run cutest_run_test.c cutest_run_mocks.h > $@

#
# Test suite executables
#
.PRECIOUS: .cutest/arg_test
.cutest/arg_test: .cutest/arg_proxified.s .cutest/arg_test_run.c .cutest/cutest_impl.o
	$(Q)$(CC) $^ -o $@ $(LDFLAGS) $(CFLAGS) -DNDEBUG -D"inline=" -I.. -I.

.PRECIOUS: .cutest/helpers_test
.cutest/helpers_test: .cutest/helpers_proxified.s .cutest/helpers_test_run.c .cutest/cutest_impl.o
	$(Q)$(CC) $^ -o $@ $(LDFLAGS) $(CFLAGS) -DNDEBUG -D"inline=" -I.. -I.

.PRECIOUS: .cutest/list_test
.cutest/list_test: .cutest/list_proxified.s .cutest/list_test_run.c .cutest/cutest_impl.o
	$(Q)$(CC) $^ -o $@ $(LDFLAGS) $(CFLAGS) -DNDEBUG -D"inline=" -I.. -I.

.PRECIOUS: .cutest/mockable_test
.cutest/mockable_test: .cutest/mockable_proxified.s .cutest/mockable_test_run.c .cutest/cutest_impl.o ../arg.o
	$(Q)$(CC) $^ -o $@ $(LDFLAGS) $(CFLAGS) -DNDEBUG -D"inline=" -I.. -I.

.PRECIOUS: .cutest/testcase_test
.cutest/testcase_test: .cutest/testcase_proxified.s .cutest/testcase_test_run.c .cutest/cutest_impl.o
	$(Q)$(CC) $^ -o $@ $(LDFLAGS) $(CFLAGS) -DNDEBUG -D"inline=" -I.. -I.

.PRECIOUS: .cutest/cutest_mock_test
.cutest/cutest_mock_test: .cutest/cutest_mock_proxified.s .cutest/cutest_mock_test_run.c .cutest/cutest_impl.o ../arg.o ../helpers.o ../mockable.o
	$(Q)$(CC) $^ -o $@ $(LDFLAGS) $(CFLAGS) -DNDEBUG -D"inline=" -I.. -I.

.PRECIOUS: .cutest/cutest_prox_test
.cutest/cutest_prox_test: .cutest/cutest_prox_proxified.s .cutest/cutest_prox_test_run.c .cutest/cutest_impl.o ../helpers.o
	$(Q)$(CC) $^ -o $@ $(LDFLAGS) $(CFLAGS) -DNDEBUG -D"inline=" -I.. -I.

.PRECIOUS: .cutest/cutest_run_test
.cutest/cutest_run_test: .cutest/cutest_run_proxified.s .cutest/cutest_run_test_run.c .cutest/cutest_impl.o ../helpers.o ../testcase.o
	$(Q)$(CC) $^ -o $@ $(LDFLAGS) $(CFLAGS) -DNDEBUG -D"inline=" -I.. -I.

#
# XML reports
#
.cutest/arg_test.res: .cutest/arg_test
	@./$< -j -n > $@

.cutest/helpers_test.res: .cutest/helpers_test
	@./$< -j -n > $@

.cutest/list_test.res: .cutest/list_test
	@./$< -j -n > $@

.cutest/mockable_test.res: .cutest/mockable_test
	@./$< -j -n > $@

.cutest/testcase_test.res: .cutest/testcase_test
	@./$< -j -n > $@

.cutest/cutest_mock_test.res: .cutest/cutest_mock_test
	@./$< -j -n > $@

.cutest/cutest_prox_test.res: .cutest/cutest_prox_test
	@./$< -j -n > $@

.cutest/cutest_run_test.res: .cutest/cutest_run_test
	@./$< -j -n > $@


#
# Wrapper target
#
.PHONY: check
check: .cutest/arg_test.res .cutest/helpers_test.res .cutest/list_test.res .cutest/mockable_test.res .cutest/testcase_test.res .cutest/cutest_mock_test.res .cutest/cutest_prox_test.res .cutest/cutest_run_test.res
	@cat $^ && echo

clean:
	$(Q)$(RM) -rf .cutest *.junit_report.xml *~
