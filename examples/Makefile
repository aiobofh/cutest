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

# Don't say anything unless I really NEED to know... Please, but I can override
# with make Q= if i would change my mind.
Q=@

# Uncomment this if you prefer clang as compiler
# CC=clang

# Disable all implicit rules
.SUFFIXES:

all: check

include ../src/cutest3.mk
ifneq (${COVERAGE},0)
include ../src/coverage.mk
endif

clean::
	$(Q)$(RM) -f *~
