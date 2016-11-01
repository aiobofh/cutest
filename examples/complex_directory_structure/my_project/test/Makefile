# Don't say anything unless I really NEED to know... Please, but I can override
# with make Q= if i would change my mind.
Q=@

CUTEST_SRC_DIR=../src

# Disable all implicit rules
.SUFFIXES:

all: check

include ../../../../src/cutest.mk

clean::
	$(Q)$(RM) -f *~
