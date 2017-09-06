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

ifneq (${Q},@)
	V=-v
else
	V=-n
endif

CUTEST_SRC_DIR ?=./
CUTEST_TEST_DIR ?=./

# Some nice flags for compiling cutest-tests with good quality
CUTEST_CFLAGS?=-g -pedantic -Wall -Wextra -std=c11
# This makes valgrind work with long double values, should suffice for
# most applications as well.
CUTEST_CFLAGS+= -mlong-double-64

ifneq (${LENIENT},0)
	CUTEST_CFLAGS+=-D"CUTEST_LENIENT_ASSERTS=1"
endif

cutest_info:
	@echo "Test-folder: $(CUTEST_TEST_DIR)"
	@echo "CUTest-path: $(CUTEST_PATH)"

cutest_turead: $(CUTEST_PATH)/cutest_turead.c
	$(Q)$(CC) $< $(CUTEST_CFLAGS) -I$(CUTEST_PATH) -o $@

# Generate a very strange C-program including cutest.h for int main().
$(CUTEST_TEST_DIR)/cutest_run.c: $(CUTEST_PATH)/cutest.h Makefile
	$(Q)echo "#include \"cutest.h\"" > $@

# Build a tool to generate a test suite runner.
$(CUTEST_TEST_DIR)/cutest_run: $(CUTEST_TEST_DIR)/cutest_run.c
	$(Q)$(CC) $< $(CUTEST_CFLAGS) -I$(CUTEST_PATH) -DCUTEST_RUN_MAIN -o $@

# Generate a very strange C-program including cutest.h for int main().
$(CUTEST_TEST_DIR)/cutest_mock.c: $(CUTEST_PATH)/cutest.h Makefile
	$(Q)echo "#include \"cutest.h\"" > $@

# Build a tool to generate a test runner program.
$(CUTEST_TEST_DIR)/cutest_mock: $(CUTEST_TEST_DIR)/cutest_mock.c
	$(Q)which cproto >/dev/null || \
	(echo "ERROR: cproto is not installed in your path"; false) && \
	$(CC) $< $(CUTEST_CFLAGS) -I$(CUTEST_PATH) -DCUTEST_MOCK_MAIN -o $@

# Generate a very strange C-program including cutest.h for int main().
$(CUTEST_TEST_DIR)/cutest_prox.c: $(CUTEST_PATH)/cutest.h Makefile
	$(Q)echo "#include \"cutest.h\"" > $@

# Build a tool to generate a test suite runner.
$(CUTEST_TEST_DIR)/cutest_prox: $(CUTEST_TEST_DIR)/cutest_prox.c
	$(Q)$(CC) $< $(CUTEST_CFLAGS) -I$(CUTEST_PATH) -DCUTEST_PROX_MAIN -o $@

# Generate a very strange C-program including cutest.h for int main().
$(CUTEST_TEST_DIR)/cutest_filt.c: $(CUTEST_PATH)/cutest.h Makefile
	$(Q)echo "#include \"cutest.h\"" > $@

# Build a tool to generate a test suite runner.
$(CUTEST_TEST_DIR)/cutest_filt: $(CUTEST_TEST_DIR)/cutest_filt.c
	$(Q)$(CC) $< $(CUTEST_CFLAGS) -I$(CUTEST_PATH) -DCUTEST_FILT_MAIN -o $@

# Extract all functions called by the design under test.
$(CUTEST_TEST_DIR)/%.tu: $(subst _test,,$(CUTEST_TEST_DIR)/%_test.c)
	$(Q)g++ -fdump-translation-unit -c $< && \
	cat $<.*.tu | c++filt > $@ && \
	rm -f $<.*.tu

# Produce an object file to be processed to search for mockable functions
.PRECIOUS: $(CUTEST_TEST_DIR)/%_mockables.o
$(CUTEST_TEST_DIR)/%_mockables.o: $(CUTEST_SRC_DIR)/%.c
	$(Q)$(CC) -o $@ -c $< $(CFLAGS) $(CUTEST_IFLAGS) -I$(CUTEST_SRC_DIR) $(CUTEST_DEFINES)

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
$(CUTEST_TEST_DIR)/%_proxified.s: $(CUTEST_TEST_DIR)/%_mockables.s $(CUTEST_TEST_DIR)/%_mockables.lst $(CUTEST_TEST_DIR)/cutest_prox $(CUTEST_TEST_DIR)/cutest_filt
	$(Q)$(CUTEST_TEST_DIR)//cutest_prox $< $(subst .s,.lst,$<) > $@

.PRECIOUS: $(CUTEST_TEST_DIR)/%_mocks.h
# Generate mocks from the call()-macro in a source-file.
$(CUTEST_TEST_DIR)/%_mocks.h: $(CUTEST_SRC_DIR)/%.c $(CUTEST_TEST_DIR)/%_mockables.lst $(CUTEST_PATH)/cutest.h $(CUTEST_TEST_DIR)/cutest_mock
	$(Q)$(CUTEST_TEST_DIR)/cutest_mock $< $(addprefix $(CUTEST_TEST_DIR)/,$(notdir $(subst .c,_mockables.lst,$<))) $(CUTEST_PATH) \
	$(CUTEST_IFLAGS) > $@; \

.PRECIOUS: $(CUTEST_TEST_DIR)/%_test_run.c
# Generate a test-runner program code from a test-source-file
$(CUTEST_TEST_DIR)/%_test_run.c: $(CUTEST_TEST_DIR)/%_test.c $(CUTEST_TEST_DIR)/%_mocks.h $(CUTEST_PATH)/cutest.h $(CUTEST_TEST_DIR)/cutest_run
	$(Q)cd $(CUTEST_TEST_DIR) && ./cutest_run $(filter-out cutest.h,$(notdir $^)) > $(notdir $@)

# Compile a test-runner from the generate test-runner program code
$(CUTEST_TEST_DIR)/%_test: $(CUTEST_TEST_DIR)/%_proxified.s $(CUTEST_TEST_DIR)/%_test_run.c
	$(Q)$(CC) -o $@ $^ $(CUTEST_CFLAGS) -I$(CUTEST_PATH) -I$(abspath $(CUTEST_TEST_DIR)) -I$(abspath $(CUTEST_SRC_DIR)) $(CUTEST_IFLAGS) -DNDEBUG -D"inline=" $(CUTEST_DEFINES) 3>&1 1>&2 2>&3 3>&- # | $(CUTEST_TEST_DIR)/cutest_filt

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

# Produce a list of potential test-suite file names
$(CUTEST_TEST_DIR)/cutest_sources.lst: $(CUTEST_SRC_DIR)
	$(Q)ls -1 $(CUTEST_SRC_DIR)/*.c | grep -v '_test.c' | grep -v 'cutest_' | grep -v '_test_run.c' | sed -e 's/^.*\///g;s/\.c/_test\.c/g' > $@

# Produce a list of actual test-suite file names
$(CUTEST_TEST_DIR)/cutest_testsuites.lst: $(CUTEST_TEST_DIR)
	$(Q)ls -1 $(CUTEST_TEST_DIR)/*_test.c | sed -e 's/^.*\///g' > $@

# Compare the potential and actual list to determine if any test-suites are missing
missing: $(CUTEST_TEST_DIR)/cutest_sources.lst $(CUTEST_TEST_DIR)/cutest_testsuites.lst
	$(Q)for i in `cat $(CUTEST_TEST_DIR)/cutest_sources.lst`; do \
	  grep $$i $(CUTEST_TEST_DIR)/cutest_testsuites.lst >/dev/null || echo "WARNING: Missing testsuite $$i" >&2; \
	done

# Compare the potential and actual list to determine if there are too many test-suites
toomany: $(CUTEST_TEST_DIR)/cutest_sources.lst $(CUTEST_TEST_DIR)/cutest_testsuites.lst
	$(Q)for i in `cat $(CUTEST_TEST_DIR)/cutest_testsuites.lst`; do \
	  grep $$i $(CUTEST_TEST_DIR)/cutest_sources.lst >/dev/null || echo "WARNING: Unused testsuite $$i" >&2; \
	done

# Run all test-suites on as many threads as needed
check:: missing toomany $(subst .c,,$(wildcard $(CUTEST_TEST_DIR)/*_test.c))
	@R=true; \
	processors=`cat /proc/cpuinfo | grep processor | wc -l`; \
	for i in $(filter-out toomany,$(filter-out missing,$^)); do \
	  while [ `ps xa | grep -v grep | grep '_test ' | wc -l` -gt $$processors ]; do sleep 0.1; done; \
	  ./$$i $V -j || rm $$i || R=false & \
	done; \
	while [ `ps xa | grep -v grep | grep '_test ' | wc -l` -gt 0 ]; do \
	  sleep 1; \
	done; \
	echo "	"; `$$R`

# Perform a memcheck on any test suite
memcheck:: $(subst .c,.memcheck,$(wildcard $(CUTEST_TEST_DIR)/*_test.c))

# Run all test-suites on as many threads as needed and verify with valgrind
valgrind:: missing toomany $(subst .c,,$(wildcard $(CUTEST_TEST_DIR)/*_test.c))
	@R=true; \
	processors=`cat /proc/cpuinfo | grep processor | wc -l`; \
	for i in $(filter-out toomany,$(filter-out missing,$^)); do \
	  while [ `ps xa | grep -v grep | grep '_test ' | wc -l` -gt $$processors ]; do sleep 0.1; done; \
	  valgrind -q ./$$i -v -j || rm $$i || R=false & \
	done; \
	while [ `ps xa | grep -v grep | grep '_test ' | wc -l` -gt 0 ]; do \
	  sleep 1; \
	done; \
	`$$R`

clean::
	$(Q)$(RM) -f $(CUTEST_TEST_DIR)/cutest_* \
	$(CUTEST_TEST_DIR)/*_test_run.c \
	$(CUTEST_TEST_DIR)/*_mocks.h \
	$(CUTEST_TEST_DIR)/*.junit_report.xml \
	$(CUTEST_SRC_DIR)/*.junit_report.xml \
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
