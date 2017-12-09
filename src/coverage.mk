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

CUTEST_CFLAGS+=-fprofile-arcs -ftest-coverage
CUTEST_COVERAGE_DIR?=.


coverage.xml: check
	$(Q)gcovr -r $(CUTEST_COVERAGE_DIR) -e 'cutest.c' -e '/usr.*' -e '.*_test.c' -e 'cutest.h' -e '.*_mocks.h' -e 'error.h' -e '.*_test_run.c' -x > $@

.NOTPARALLEL: lines.cov
lines.cov: $(subst .c,,$(wildcard $(CUTEST_TEST_DIR)/*_test.c))
	$(Q)gcovr -r $(CUTEST_COVERAGE_DIR) -e 'cutest.c' -e '/usr.*' -e '.*_test.c' -e '.*cutest.h' -e '.*_mocks.h' -e 'error.h' -e '.*_test_run.c' $(CUTEST_COVERAGE_EXCLUDE) | egrep -v '^File' | egrep -v '^-' | egrep -v '^Directory' | grep -v 'GCC Code' | grep -v '100%' | grep -v "\-\-\%" > $@; true

.NOTPARALLEL: branches.cov
branches.cov: $(subst .c,,$(wildcard $(CUTEST_TEST_DIR)/*_test.c))
	$(Q)gcovr -r $(CUTEST_COVERAGE_DIR) -e 'cutest.c' -e '/usr.*' -e '.*_test.c' -e '.*cutest.h'  -e '.*_mocks.h' -e 'error.h' -e '.*_test_run.c' $(CUTEST_COVERAGE_EXCLUDE) -b | egrep -v '^File' | egrep -v '^-' | egrep -v '^Directory' | grep -v 'GCC Code' | grep -v '100%' | grep -v "\-\-\%" > $@; true

output_coverage: lines.cov branches.cov
	$(Q)test -s lines.cov && echo "Lines not covered:" >&2 && cat lines.cov >&2 && echo >&2; \
	test -s branches.cov && echo "Branches not covered:" >&2; cat branches.cov >&2

check::
	$(Q)$(MAKE) -r --no-print-directory output_coverage

valgrind::
	$(Q)$(MAKE) -r --no-print-directory output_coverage

clean::
	$(Q)rm -rf coverage.xml *.gcno *.gcda *.cov
