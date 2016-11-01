# Build everything in src

info:
	echo $(MAKEFLAGS)

all:
	@make -s -C src

check:
	@make -s -C test

clean::
	@rm -f *~; make -s -C src clean; make -s -C test clean
