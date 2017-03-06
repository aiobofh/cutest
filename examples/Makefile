# Don't say anything unless I really NEED to know... Please, but I can override
# with make Q= if i would change my mind.
Q=@

# Disable all implicit rules
.SUFFIXES:

all: check

include ../src/cutest.mk
include ../src/coverage.mk

clean::
	$(Q)$(RM) -f *~
