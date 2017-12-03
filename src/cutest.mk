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

CUTEST_PATH := $(subst /cutest.mk,,$(abspath $(lastword $(MAKEFILE_LIST))))

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

# Some nice flags for compiling cutest-tests with good quality
ifneq ($(CC),clang)
	CUTEST_CFLAGS?=-g -pedantic -Wall -Wextra -std=c11
else
	CUTEST_CFLAGS?=-pedantic -Wall -Wextra -std=c11
endif
# This makes valgrind work with long double values, should suffice for
# most applications as well.
HAS_LONGOPT=$(shell $(CC) -mlong-double-64 2>&1 | grep 'unrecognized' >/dev/null && echo "yes")
ifneq ($(CC),clang)
	ifneq ($(HAS_LONGOPT),yes)
		CUTEST_CFLAGS+= -mlong-double-64
	endif
endif

ifneq (${LENIENT},0)
	CUTEST_CFLAGS+=-D"CUTEST_LENIENT_ASSERTS=1"
endif

export ASAN_OPTIONS=strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1
export UBSAN_OPTIONS=print_stacktrace=1

CUTEST_CFLAGS+=-Wno-pragmas

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

SOURCES=$(filter-out %_test.c,$(wildcard $(CUTEST_SRC_DIR)/*.c))
EXPECTED_TEST_SUITES=$(sort $(subst .c,_test.c,$(SOURCES)))
FOUND_TEST_SUITES=$(sort $(wildcard $(CUTEST_SRC_DIR)/*_test.c))
MISSING_TEST_SUITES=$(filter-out $(FOUND_TEST_SUITES),$(EXPECTED_TEST_SUITES))
MISSING_SOURCES=$(subst _test.c,.c,$(filter-out $(EXPECTED_TEST_SUITES),$(FOUND_TEST_SUITES)))

cutest_info:
	@echo "Test-folder          : $(CUTEST_TEST_DIR)"
	@echo "Source-folder        : $(CUTEST_SRC_DIR)"
	@echo "CUTest-path          : $(CUTEST_PATH)"
	@echo "CUTest-CFLAGS        : $(CUTEST_CFLAGS)"
	@echo "CC                   : $(CC) $(findstring gcc,$(CC))"
	@echo "Sources to test      : $(SOURCES)"
	@echo "Expected tests suites: $(EXPECTED_TEST_SUITES)"
	@echo "Found tests suites   : $(FOUND_TEST_SUITES)"
	@echo "Missing sources      : $(MISSING_SOURCES)"
	@echo "Missing test suites  : $(MISSING_TEST_SUITES)"

cutest_turead: $(CUTEST_PATH)/cutest_turead.c
	$(Q)$(CC) $< $(CUTEST_CFLAGS) -I$(CUTEST_PATH) -o $@

# Build a tool to generate a test suite runner.
$(CUTEST_PATH)/cutest_run: $(CUTEST_PATH)/cutest_run.c $(CUTEST_PATH)/helpers.c
	$(Q)$(CC) $^ -O2 $(CUTEST_CFLAGS) -I$(CUTEST_PATH) -o $@

# Build a tool to generate a test suite runner.
$(CUTEST_PATH)/cutest_mock: $(CUTEST_PATH)/cutest_mock.c $(CUTEST_PATH)/helpers.c $(CUTEST_PATH)/mockable.c $(CUTEST_PATH)/arg.c
	$(Q)$(CC) $^ -O2 $(CUTEST_CFLAGS) -I$(CUTEST_PATH) -o $@

# Build a tool to generate an assembler file with replaced branches/calls.
$(CUTEST_PATH)/cutest_prox: $(CUTEST_PATH)/cutest_prox.c $(CUTEST_PATH)/helpers.c
	$(Q)$(CC) $^ -O2 $(CUTEST_CFLAGS) -I$(CUTEST_PATH) -o $@

# Produce an object file to be processed to search for mockable functions
.PRECIOUS: $(CUTEST_TEST_DIR)/%_mockables.o
$(CUTEST_TEST_DIR)/%_mockables.o: $(CUTEST_SRC_DIR)/%.c
	$(Q)$(CC) -o $@ -c $< $(CFLAGS) $(CUTEST_IFLAGS) -I$(CUTEST_SRC_DIR) $(CUTEST_DEFINES) -fvisibility=hidden -fno-inline -g -D"inline="

# Generate a list of all posible mockable functions
.PRECIOUS: $(CUTEST_TEST_DIR)/%_mockables.lst
$(CUTEST_TEST_DIR)/%_mockables.lst: $(CUTEST_TEST_DIR)/%_mockables.o
	$(Q)nm $< | sed 's/.* //g' | grep -v '__stack_' | sort -u > $@

# Generate an assembler file for later processing
.PRECIOUS: $(CUTEST_TEST_DIR)/%_mockables.s
$(CUTEST_TEST_DIR)/%_mockables.s: $(CUTEST_SRC_DIR)/%.c
	$(Q)$(CC) -S -fverbose-asm -fvisibility=hidden -fno-inline -g -O0 \
	-o $@ -c $^ $(CUTEST_CFLAGS) $(CUTEST_IFLAGS) $(CUTEST_DEFINES) -D"static=" -D"inline=" -D"main=MAIN"

# Generate an assembler output with all function calls replaced to cutest mocks/stubs
.PRECIOUS: $(CUTEST_TEST_DIR)/%_proxified.s
$(CUTEST_TEST_DIR)/%_proxified.s: $(CUTEST_TEST_DIR)/%_mockables.s $(CUTEST_TEST_DIR)/%_mockables.lst $(CUTEST_PATH)/cutest_prox
	$(Q)$(CUTEST_PATH)/cutest_prox $< $(subst .s,.lst,$<) > $@

.PRECIOUS: $(CUTEST_TEST_DIR)/%_mocks.h
# Generate mocks from the call()-macro in a source-file.
$(CUTEST_TEST_DIR)/%_mocks.h: $(CUTEST_SRC_DIR)/%.c $(CUTEST_TEST_DIR)/%_mockables.lst $(CUTEST_PATH)/cutest.h $(CUTEST_PATH)/cutest_mock
	$(Q)$(CUTEST_PATH)/cutest_mock $(wordlist 1,3,$^) $(CUTEST_PATH) $(CUTEST_IFLAGS) > $@

.PRECIOUS: $(CUTEST_TEST_DIR)/%_test_run.c
# Generate a test-runner program code from a test-source-file
$(CUTEST_TEST_DIR)/%_test_run.c: $(CUTEST_TEST_DIR)/%_test.c $(CUTEST_TEST_DIR)/%_mocks.h $(CUTEST_PATH)/cutest.h $(CUTEST_PATH)/cutest_run
	$(Q)$(CUTEST_PATH)/cutest_run $(wordlist 1,2,$^) > $@

# Compile a test-runner from the generate test-runner program code
$(CUTEST_TEST_DIR)/%_test: $(CUTEST_TEST_DIR)/%_proxified.s $(CUTEST_TEST_DIR)/%_test_run.c
	$(Q)$(CC) -o $@ $^ $(CUTEST_CFLAGS) -I$(CUTEST_PATH) -I$(abspath $(CUTEST_TEST_DIR)) -I$(abspath $(CUTEST_SRC_DIR)) $(CUTEST_IFLAGS) -DNDEBUG -D"inline=" $(CUTEST_DEFINES) 3>&1 1>&2 2>&3 3>&-

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

# Run all test-suites on as many threads as needed
check:: $(subst .c,,$(wildcard $(CUTEST_TEST_DIR)/*_test.c))
	@R=true; \
	processors=`cat /proc/cpuinfo | grep processor | wc -l`; \
	for i in $^; do \
	  while [ `ps xa | grep -v grep | grep '_test ' | wc -l` -gt $$processors ]; do sleep 0.1; done; \
	  ./$$i $V -j -s || rm $$i || R=false & \
	done; \
	while [ `ps xa | grep -v grep | grep '_test ' | wc -l` -gt 0 ]; do \
	  sleep 1; \
	done; \
	echo "	"; `$$R`
ifneq ($(MISSING_SOURCES),)
	$(warning "Missing source(s) $(MISSING_SOURCES) - Did you delete the test?")
endif
ifneq ($(MISSING_TEST_SUITES),)
	$(warning "Missing test-suite(s) $(MISSING_TEST_SUITES) - Did you forget to write the test suites?")
endif

sanitize: check

# Perform a memcheck on any test suite
memcheck:: $(subst .c,.memcheck,$(wildcard $(CUTEST_TEST_DIR)/*_test.c))

# Run all test-suites on as many threads as needed and verify with valgrind
valgrind:: $(subst .c,,$(wildcard $(CUTEST_TEST_DIR)/*_test.c))
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

clean_cutest:
	$(Q)$(RM) $(CUTEST_PATH)/cutest_run \
	$(CUTEST_PATH)/cutest_prox \
	$(CUTEST_PATH)/cutest_mock

clean::
	$(Q)$(RM) -f $(CUTEST_TEST_DIR)/*_test_run.c \
	$(CUTEST_TEST_DIR)/cutest_run \
	$(CUTEST_TEST_DIR)/cutest_mock \
	$(CUTEST_TEST_DIR)/cutest_prox \
	$(CUTEST_TEST_DIR)/cutest_filt \
	$(CUTEST_TEST_DIR)/cutest_filt.c \
	$(CUTEST_TEST_DIR)/*_mocks.h \
	$(CUTEST_TEST_DIR)/*.junit_report.xml \
	$(CUTEST_SRC_DIR)/*.junit_report.xml \
	$(CUTEST_SRC_DIR)/default.profraw \
	$(CUTEST_TEST_DIR)/*.memcheck \
	$(CUTEST_TEST_DIR)/*_test \
	$(CUTEST_TEST_DIR)/*_test.exe \
	$(CUTEST_TEST_DIR)/*.tu \
	$(CUTEST_TEST_DIR)/*_mockables.* \
	$(CUTEST_TEST_DIR)/*_proxified.* \
	$(CUTEST_TEST_DIR)/cutest_help.rst \
	$(CUTEST_TEST_DIR)/cutest_help.html \
	$(CUTEST_TEST_DIR)/cutest_sources.lst \
	$(CUTEST_TEST_DIR)/cutest_testsuites.lst \
	$(CUTEST_TEST_DIR)/*~
