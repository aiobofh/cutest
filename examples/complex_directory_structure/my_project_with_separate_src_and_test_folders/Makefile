# Build everything in src

all: check src

info:
	echo $(MAKEFLAGS)

src:
	@make -s -C src

check:
	@make -s -C test

clean::
	@rm -f *~; make -s -C src clean; make -s -C test clean
