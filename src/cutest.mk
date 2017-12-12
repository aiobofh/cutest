#
# cutest.mk
#
#    CUTest Makefile - To be included in your build system
#    Copyright (C) 2017 Joakim Ekblad - AiO Secure Teletronics
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# Reuse the Q variable for making cutest test running verbose too.
Q ?=@

export CC

CUTEST_PATH := $(abspath $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

CUTEST_RUN=$(CUTEST_PATH)/cutest_run
CUTEST_MOCK=$(CUTEST_PATH)/cutest_mock
CUTEST_PROX=$(CUTEST_PATH)/cutest_prox
CUTEST_WORK=$(CUTEST_PATH)/cutest_work

include $(CUTEST_PATH)/cproto.mk

ifeq ($(MAKECMDGOALS),sanitize)
	CC=clang
endif

ifneq (${Q},@)
	V=-v
else
	V=-n
endif

CUTEST_SRC_DIR ?=./
CUTEST_TEST_DIR ?=./

CUTEST_SRC_DIR:=$(abspath $(CUTEST_SRC_DIR))
CUTEST_TEST_DIR:=$(abspath $(CUTEST_TEST_DIR))

PEDANTIC:=-pedantic -Wall
# Some nice flags for compiling cutest-tests with good quality
ifneq ($(findstring clang,$(CC)),clang)
	CUTEST_CFLAGS?=-g
else
	CUTEST_CFLAGS?=
endif

HAS_VISIBILITY_HIDDEN:=$(shell $(CC) -o $(CUTEST_PATH)/empty $(CUTEST_PATH)/empty.c -fvisibility=hidden 2>&1 >/dev/null && echo "yes")
ifeq ("$(HAS_VISIBILITY_HIDDEN)","yes")
	VISIBILITY_HIDDEN:=-fvisibility=hidden
else
	VISIBILITY_HIDDEN:=
endif

HAS_COV:=$(shell $(CC) -o $(CUTEST_PATH)/empty $(CUTEST_PATH)/empty.c -fprofile-arcs 2>&1 >/dev/null && echo "yes")
ifeq ("$(HAS_COV)","yes")
	COV:=-fprofile-arcs -ftest-coverage
else
	COV:=
endif

HAS_NOPRAGMA:=$(shell $(CC) -o $(CUTEST_PATH)/empty $(CUTEST_PATH)/empty.c -Wno-pragma 2>&1 >/dev/null && echo "yes")
ifeq ("$(HAS_NOPRAGMA)","yes")
	NOPRAGMA:=-Wno-pragma
else
	NOPRAGMA:=
endif

HAS_WEXTRA:=$(shell $(CC) -o $(CUTEST_PATH)/empty $(CUTEST_PATH)/empty.c -Wextra 2>&1 >/dev/null && echo "yes")
ifeq ("$(HAS_WEXTRA)","yes")
	WEXTRA:=-Wextra
else
	WEXTRA:=
endif

VARIADIC:=none
HAS_VARIADIC:=$(shell $(CC) -o $(CUTEST_PATH)/empty $(CUTEST_PATH)/empty.c -DVARIADIC=1 2>&1 >/dev/null && echo "yes")
ifeq ("$(HAS_VARIADIC)","yes")
	VARIADIC:=-D"VARIADIC=1"
else
	VARIADIC:=
endif

STD:=none
HAS_C11:=$(shell $(CC) -std=c11 -o $(CUTEST_PATH)/empty $(CUTEST_PATH)/empty.c 2>&1 >/dev/null && echo "yes")
ifeq ("$(HAS_C11)","yes")
	STD:=-std=c11
else
	HAS_C11:=no
	HAS_C99:=$(shell $(CC) -std=c99 -o $(CUTEST_PATH)/empty $(CUTEST_PATH)/empty.c 2>&1 >/dev/null && echo "yes")
	ifeq ("$(HAS_C99)","yes")
		STD:=-std=c99
	else
		HAS_C99:=no
		HAS_C90:=$(shell $(CC) -std=c90 -o $(CUTEST_PATH)/empty $(CUTEST_PATH)/empty.c 2>&1 >/dev/null && echo "yes")
		ifeq ("$(HAS_C90)","yes")
			STD:=-std=c90
		else
			PEDANTIC:=
			STD:=
		endif
	endif
endif

all:
	$(Q)$(MAKE) -s -r --no-print-directory cutest_run && \
	$(MAKE) -s -r --no-print-directory cutest_mock && \
	$(MAKE) -s -r --no-print-directory cutest_prox && \
	$(MAKE) -s -r --no-print-directory cutest_work

# This makes valgrind work with long double values, should suffice for
# most applications as well.
HAS_LONGOPT:=$(shell $(CC) -o $(CUTEST_PATH)/empty $(CUTEST_PATH)/empty.c -mlong-double-64 2>&1 >/dev/null && echo "yes")
ifeq ("$(HAS_LONGOPT)","yes")
	LONG_DOUBLE_64:=-mlong-double-64
else
	LONG_DOUBLE_64:=
endif

CUTEST_CFLAGS+=$(LONG_DOUBLE_64) $(STD) $(VARIADIC) $(WEXTRA) $(NOPRAGMA) $(PEDANTIC)

ifneq (${LENIENT},0)
	ifeq ("$(STD)","-std=c11")
		CUTEST_CFLAGS+=-D"CUTEST_LENIENT_ASSERTS=1"
	endif
endif

export ASAN_OPTIONS=strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1
export UBSAN_OPTIONS=print_stacktrace=1

#CUTEST_CFLAGS+=-Wno-pragmas

ifeq ($(findstring gcc,$(CC)),gcc)
	CUTEST_CFLAGS+=-D"CUTEST_GCC=1"
endif
ifeq ($(findstring clang,$(CC)),clang)
	CUTEST_CFLAGS+=-D"CUTEST_CLANG=1"
endif

ifeq ($(MAKECMDGOALS),sanitize)
	CUTEST_CFLAGS+=-fsanitize=address,leak,undefined -fno-omit-frame-pointer
	ASAN_OPTIONS=detect_leaks=1
	export ASAN_OPTIONS
endif

SOURCES=$(notdir $(filter-out %_test_run.c,$(filter-out %_test.c,$(wildcard $(CUTEST_SRC_DIR)/*.c))))
EXPECTED_TEST_SUITES=$(sort $(notdir $(subst .c,_test.c,$(filter-out cutest.c,$(SOURCES)))))
FOUND_TEST_SUITES=$(sort $(notdir $(wildcard $(CUTEST_TEST_DIR)/*_test.c)))
MISSING_TEST_SUITES=$(filter-out $(FOUND_TEST_SUITES),$(EXPECTED_TEST_SUITES))
MISSING_SOURCES=$(subst _test.c,.c,$(filter-out $(EXPECTED_TEST_SUITES),$(FOUND_TEST_SUITES)))

cutest_info:
	@echo "Current path         : $(abspath .)"
	@echo "Test-folder          : $(CUTEST_TEST_DIR)"
	@echo "Source-folder        : $(CUTEST_SRC_DIR)"
	@echo "CUTest-path          : $(CUTEST_PATH)"
	@echo "CUTest-CFLAGS        : $(CUTEST_CFLAGS)"
	@echo "CC                   : $(CC) '$(findstring gcc,$(CC))'"
	@echo "C-standard           : $(STD)"
	@echo "LTO                  : $(LTO)"
	@echo "CLFAGS               : $(CUTEST_CFLAGS)"
#	@echo "Installed cproto     : $(INSTALLED_CPROTO) $(INSTALLED_CPROTO_VER)"
	@echo "Using cproto         : $(CPROTO)"
	@echo "Cproto-archive       : $(CPROTO_PATH).tgz"
	@echo "Sources to test      : $(SOURCES)"
	@echo "Expected tests suites: $(EXPECTED_TEST_SUITES)"
	@echo "Found tests suites   : $(FOUND_TEST_SUITES)"
	@echo "Missing sources      : $(MISSING_SOURCES)"
	@echo "Missing test suites  : $(MISSING_TEST_SUITES)"

$(CUTEST_PATH)/cutest.o: $(CUTEST_PATH)/cutest.c
	$(Q)$(CC) -c $^ $(CUTEST_CFLAGS) -I$(CUTEST_PATH) -I$(abspath $(CUTEST_SRC_DIR)) $(CUTEST_IFLAGS) -DNDEBUG -D"inline=" $(CUTEST_DEFINES) -o $@

# Build a tool to generate a test suite runner.
$(CUTEST_RUN):
	$(Q)$(MAKE) -s -r --no-print-directory -C $(CUTEST_PATH) cutest_run

# Build a tool to generate a test suite runner.
$(CUTEST_MOCK):
	$(Q)$(MAKE) -s -r --no-print-directory -C $(CUTEST_PATH) cutest_mock

# Build a tool to generate an assembler file with replaced branches/calls.
$(CUTEST_PROX):
	$(Q)$(MAKE) -s -r --no-print-directory -C $(CUTEST_PATH) cutest_prox

# Build a tool to generate an assembler file with replaced branches/calls.
$(CUTEST_WORK):
	$(Q)$(MAKE) -s -r --no-print-directory -C $(CUTEST_PATH) cutest_work

# Produce an object file to be processed to search for mockable functions
.PRECIOUS: $(CUTEST_TEST_DIR)/%_mockables.o
$(CUTEST_TEST_DIR)/%_mockables.o: $(CUTEST_SRC_DIR)/%.c
	$(Q)$(CC) -o $@ -c $< $(CFLAGS) $(CUTEST_IFLAGS) -I$(CUTEST_SRC_DIR) $(CUTEST_DEFINES) $(VISIBILITY_HIDDEN) -fno-inline -g -D"inline="

# Generate a list of all posible mockable functions
.PRECIOUS: $(CUTEST_TEST_DIR)/%_mockables.lst
$(CUTEST_TEST_DIR)/%_mockables.lst: $(CUTEST_TEST_DIR)/%_mockables.o
	$(Q)nm $< | sed 's/.* //g' | grep -v '__stack_' | sort -u > $@ && \
	grep 'gcc2_compiled.' >/dev/null $@ && sed -i 's/^_//g' $@ || true

# Generate an assembler file for later processing
.PRECIOUS: $(CUTEST_TEST_DIR)/%_mockables.s
$(CUTEST_TEST_DIR)/%_mockables.s: $(CUTEST_SRC_DIR)/%.c
	$(Q)$(CC) -S -fverbose-asm $(VISIBILITY_HIDDEN) -fno-inline -g -O0 \
	-o $@ -c $^ $(CUTEST_CFLAGS) $(CUTEST_IFLAGS) $(CUTEST_DEFINES) -D"static=" -D"inline=" -D"main=MAIN"

# Generate an assembler output with all function calls replaced to cutest mocks/stubs
.PRECIOUS: $(CUTEST_TEST_DIR)/%_proxified.s
$(CUTEST_TEST_DIR)/%_proxified.s: $(CUTEST_TEST_DIR)/%_mockables.s $(CUTEST_TEST_DIR)/%_mockables.lst $(CUTEST_PROX)
	$(Q)$(CUTEST_PROX) $< $(subst .s,.lst,$<) > $@

.PRECIOUS: $(CUTEST_TEST_DIR)/%_mocks.h
# Generate mocks from the call()-macro in a source-file.
$(CUTEST_TEST_DIR)/%_mocks.h: $(CUTEST_SRC_DIR)/%.c $(CUTEST_TEST_DIR)/%_mockables.lst $(CUTEST_PATH)/cutest.h $(CUTEST_MOCK) $(CPROTO)
	$(Q)$(CUTEST_MOCK) $(CPROTO) $(wordlist 1,2,$^) $(CUTEST_PATH) $(CUTEST_IFLAGS) > $@

.PRECIOUS: $(CUTEST_TEST_DIR)/%_test_run.c
# Generate a test-runner program code from a test-source-file
$(CUTEST_TEST_DIR)/%_test_run.c: $(CUTEST_TEST_DIR)/%_test.c $(CUTEST_TEST_DIR)/%_mocks.h $(CUTEST_PATH)/cutest.h $(CUTEST_RUN)
	$(Q)$(CUTEST_RUN) $(wordlist 1,2,$^) > $@

# Compile a test-runner from the generate test-runner program code
$(CUTEST_TEST_DIR)/%_test: $(CUTEST_TEST_DIR)/%_proxified.s $(CUTEST_TEST_DIR)/%_test_run.c $(CUTEST_PATH)/cutest.o
	$(Q)$(CC) -o $@ $^ $(LTO) $(CUTEST_CFLAGS) -I$(CUTEST_PATH) -I$(abspath $(CUTEST_TEST_DIR)) -I$(abspath $(CUTEST_SRC_DIR)) $(CUTEST_IFLAGS) -DNDEBUG -D"inline=" $(CUTEST_DEFINES) 3>&1 1>&2 2>&3 3>&-

# Print the CUTest manual
$(CUTEST_TEST_DIR)/cutest_help.rst: $(CUTEST_PATH)/cutest.h
	$(Q)grep -e '^ * ' $< | \
	grep -v '*/' | \
	grep -v -e '^  ' | \
	sed -e 's/^ \* //g;s/^ \*//g' > $@

# Output the CUTest manual as HTML
$(CUTEST_TEST_DIR)/cutest_help.html: $(CUTEST_TEST_DIR)/cutest_help.rst
	$(Q)rst2html $< > $@

# Show the CUTest manual
cutest_help: $(CUTEST_TEST_DIR)/cutest_help.rst
	$(Q)less $<

# Produce a valgrind report
$(CUTEST_TEST_DIR)/%_test.memcheck: $(CUTEST_TEST_DIR)/%_test
	$(Q)valgrind -q --xml=yes --xml-file=$@ ./$< > /dev/null
ifneq ($(MISSING_SOURCES),)
	$(warning "Missing source(s) $(MISSING_SOURCES) - Did you delete the test?")
endif
ifneq ($(MISSING_TEST_SUITES),)
	$(warning "Missing test-suite(s) $(MISSING_TEST_SUITES) - Did you forget to write the test suites?")
endif

# If using CUTest without the cutest_work program to parallelize
$(CUTEST_TEST_DIR)/%_test.junit_report.xml: $(CUTEST_TEST_DIR)/%_test
	$(Q)$^ $V -j -s 1> $(subst _test,_test.stdout,$^) 2> $(subst _test,_test.stderr,$^)

# Run all test-suites on as many threads as needed gnu make
makecheck:: $(subst .c,.junit_report.xml,$(wildcard $(CUTEST_TEST_DIR)/*_test.c))
ifneq ($(MISSING_SOURCES),)
	$(warning "Missing source(s) $(MISSING_SOURCES) - Did you delete the test?")
endif
ifneq ($(MISSING_TEST_SUITES),)
	$(warning "Missing test-suite(s) $(MISSING_TEST_SUITES) - Did you forget to write the test suites?")
endif
	$(Q)echo

# Run all test-suites on as many threads as needed using cutest_work
check:: $(subst .c,,$(wildcard $(CUTEST_TEST_DIR)/*_test.c)) $(CUTEST_WORK)
ifneq ($(MISSING_SOURCES),)
	$(warning "Missing source(s) $(MISSING_SOURCES) - Did you delete the test?")
endif
ifneq ($(MISSING_TEST_SUITES),)
	$(warning "Missing test-suite(s) $(MISSING_TEST_SUITES) - Did you forget to write the test suites?")
endif
	$(Q)$(CUTEST_WORK) $V $(filter-out $(CUTEST_WORK),$^)

sanitize: check

# Perform a memcheck on any test suite
memcheck:: $(subst .c,.memcheck,$(wildcard $(CUTEST_TEST_DIR)/*_test.c))

valgrind:: $(subst .c,,$(wildcard $(CUTEST_TEST_DIR)/*_test.c)) $(CUTEST_PATH)/cutest_work
ifneq ($(MISSING_SOURCES),)
	$(warning "Missing source(s) $(MISSING_SOURCES) - Did you delete the test?")
endif
ifneq ($(MISSING_TEST_SUITES),)
	$(warning "Missing test-suite(s) $(MISSING_TEST_SUITES) - Did you forget to write the test suites?")
endif
#	$(Q)$(CUTEST_PATH)/cutest_work -V $(addprefix $(CUTEST_TEST_DIR)/,$(filter-out $(CUTEST_PATH)/cutest_work,$^))
	$(Q)$(CUTEST_PATH)/cutest_work -V $(filter-out $(CUTEST_PATH)/cutest_work,$^)

# Run all test-suites on as many threads as needed and verify with valgrind
makevalgrind:: $(subst .c,,$(wildcard $(CUTEST_TEST_DIR)/*_test.c))
	@R=true; \
	processors=`cat /proc/cpuinfo | grep processor | wc -l`; \
	for i in $^; do \
	  while [ `ps xa | grep -v grep | grep '_test ' | wc -l` -gt $$processors ]; do sleep 0.1; done; \
	  valgrind --track-origins=yes -q ./$$i -v -j -s || rm $$i || R=false & \
	done; \
	while [ `ps xa | grep -v grep | grep '_test ' | wc -l` -gt 0 ]; do \
	  sleep 1; \
	done; \
	`$$R`

#
# Dependencies described for test cases
#
$(CUTEST_TEST_DIR)/cutest_run_test:: helpers.c testcase.c

$(CUTEST_TEST_DIR)/cutest_mock_test:: helpers.c mockable.c arg.c

$(CUTEST_TEST_DIR)/cutest_prox_test:: helpers.c

$(CUTEST_TEST_DIR)/cutest_work_test:: helpers.c

$(CUTEST_TEST_DIR)/mockable_test:: arg.c list.c

clean_cutest:
	$(Q)$(MAKE) -s -r --no-print-directory -C $(CUTEST_PATH) -f $(CUTEST_PATH)/Makefile clean

clean::
	$(Q)$(RM) -f $(CUTEST_TEST_DIR)/*_test_run.c \
	$(CUTEST_PATH)/empty \
	$(CUTEST_PATH)/cutest.o \
	$(CUTEST_TEST_DIR)/cutest_run \
	$(CUTEST_TEST_DIR)/cutest_mock \
	$(CUTEST_TEST_DIR)/cutest_prox \
	$(CUTEST_TEST_DIR)/cutest_work \
	$(CUTEST_TEST_DIR)/cutest_filt \
	$(CUTEST_TEST_DIR)/cutest_filt.c \
	$(CUTEST_TEST_DIR)/*_mocks.h \
	$(CUTEST_TEST_DIR)/*.junit_report.xml \
	$(CUTEST_SRC_DIR)/*.junit_report.xml \
	$(CUTEST_SRC_DIR)/default.profraw \
	$(CUTEST_TEST_DIR)/*.memcheck \
	$(CUTEST_TEST_DIR)/*_test \
	$(CUTEST_TEST_DIR)/*_test.stderr \
	$(CUTEST_TEST_DIR)/*_test.stdout \
	$(CUTEST_TEST_DIR)/*_test.exe \
	$(CUTEST_TEST_DIR)/*.tu \
	$(CUTEST_TEST_DIR)/*_mockables.* \
	$(CUTEST_TEST_DIR)/*_proxified.* \
	$(CUTEST_TEST_DIR)/cutest_help.rst \
	$(CUTEST_TEST_DIR)/cutest_help.html \
	$(CUTEST_TEST_DIR)/cutest_sources.lst \
	$(CUTEST_TEST_DIR)/cutest_testsuites.lst \
	$(CUTEST_TEST_DIR)/*_test.log \
	$(CUTEST_TEST_DIR)/*~ \
	$(CUTEST_PATH)/*.uaem \
	$(CUTEST_SRC_DIR)/*.uaem \
	$(CUTEST_PATH)/*.gcno \
	$(CUTEST_SRC_DIR)/*.gcno \
	$(CUTEST_PATH)/*.gcda \
	$(CUTEST_SRC_DIR)/*.gcda \
	$(CUTEST_TEST_DIR)/*.uaem && \
	$(RM) -rf $(CUTEST_PATH)/.terminfo
