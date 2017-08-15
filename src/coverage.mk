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
	$(Q)gcovr -r $(CUTEST_COVERAGE_DIR) -e '/usr.*' -e '.*_test.c' -e 'cutest.h' -e '.*_mocks.h' -e 'error.h' -e '.*_test_run.c' -x > $@

check::
	$(Q)echo "Lines covered:"; \
	gcovr -r $(CUTEST_COVERAGE_DIR) -e '/usr.*' -e '.*_test.c' -e '.*cutest.h' -e '.*_mocks.h' -e 'error.h' -e '.*_test_run.c' $(CUTEST_COVERAGE_EXCLUDE) | egrep -v '^File' | egrep -v '^-' | egrep -v '^Directory' | grep -v 'GCC Code'; \
	echo ""; \
	echo "Branches covered:"; \
	gcovr -r $(CUTEST_COVERAGE_DIR) -e '/usr.*' -e '.*_test.c' -e '.*cutest.h'  -e '.*_mocks.h' -e 'error.h' -e '.*_test_run.c' $(CUTEST_COVERAGE_EXCLUDE) -b | egrep -v '^File' | egrep -v '^-' | egrep -v '^Directory' | grep -v 'GCC Code';

clean::
	$(Q)rm -rf coverage.xml *.gcno *.gcda
