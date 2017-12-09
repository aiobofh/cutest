#
# Makefile
#
#    CUTest Example Makefile - To demonstrate a way to use CUTest
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

all: check src

info:
	echo $(MAKEFLAGS)

src:
	@mkdir obj >/dev/null; make -s -C src

cutest_info:
	@$(MAKE) -r --no-print-directory -C test cutest_info

check:
	@$(MAKE) -r --no-print-directory -C test

valgrind:
	@$(MAKE) -r --no-print-directory -C test valgrind

sanitize:
	@$(MAKE) -r --no-print-directory -C test sanitize

clean_cutest:
	@$(MAKE) -r --no-print-directory -C test clean_cutest

clean::
	@rm -f *~ obj; \
	$(MAKE) -r --no-print-directory -C src clean; \
	$(MAKE) -r --no-print-directory -C test clean
