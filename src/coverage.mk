#
# coverage.mk
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

Q ?=@

.SUFFIXES:

ifeq ("$(HAS_COV)","")
	HAS_COV:=$(shell $(CC) -o $(CUTEST_PATH)/empty $(CUTEST_PATH)/empty.c -fprofile-arcs 2>&1 >/dev/null && echo "yes")
endif

ifeq ("$(HAS_COV)","yes")
CUTEST_CFLAGS+=-fprofile-arcs -ftest-coverage
CUTEST_COVERAGE_DIR?=.

COVERAGE_XML:=$(CUTEST_TEST_DIR)/coverage.xml
LINES_COV:=$(CUTEST_TEST_DIR)/lines.cov
BRANCHES_COV:=$(CUTEST_TEST_DIR)/branches.cov

$(COVERAGE_XML): check
	$(Q)gcovr -r $(CUTEST_COVERAGE_DIR) -e 'cutest.c' -e '/usr.*' -e '.*_test.c' -e 'cutest.h' -e '.*_mocks.h' -e 'error.h' -e '.*_test_run.c' -x > $@

.NOTPARALLEL: $(LINES_COV)
$(LINES_COV): $(subst .c,,$(wildcard $(CUTEST_TEST_DIR)/*_test.c))
	$(Q)gcovr -r $(CUTEST_COVERAGE_DIR) -e 'cutest.c' -e '/usr.*' -e '.*_test.c' -e '.*cutest.h' -e '.*_mocks.h' -e 'error.h' -e '.*_test_run.c' $(CUTEST_COVERAGE_EXCLUDE) | egrep -v '^File' | egrep -v '^-' | egrep -v '^Directory' | grep -v 'GCC Code' | grep -v '100%' | grep -v "\-\-\%" > $@; true

.NOTPARALLEL: $(BRANCHES_COV)
$(BRANCHES_COV): $(subst .c,,$(wildcard $(CUTEST_TEST_DIR)/*_test.c))
	$(Q)gcovr -r $(CUTEST_COVERAGE_DIR) -e 'cutest.c' -e '/usr.*' -e '.*_test.c' -e '.*cutest.h'  -e '.*_mocks.h' -e 'error.h' -e '.*_test_run.c' $(CUTEST_COVERAGE_EXCLUDE) -b | egrep -v '^File' | egrep -v '^-' | egrep -v '^Directory' | grep -v 'GCC Code' | grep -v '100%' | grep -v "\-\-\%" > $@; true

output_coverage: $(LINES_COV) $(BRANCHES_COV)
	$(Q)test -s $(LINES_COV) && echo "Lines not covered:" >&2 && cat $(LINES_COV) >&2 && echo >&2; \
	test -s $(BRANCHES_COV) && echo "Branches not covered:" >&2; cat $(BRANCHES_COV) >&2

check::
	$(Q)$(MAKE) -r --no-print-directory output_coverage

valgrind::
	$(Q)$(MAKE) -r --no-print-directory output_coverage

endif
clean::
	$(Q)$(RM) $(COVERAGE_XML) $(CUTEST_TEST_DIR)/*.gcno $(CUTEST_TEST_DIR)/*.gcda $(LINES_COV) $(BRANCHES_COV)
