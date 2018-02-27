Q ?=@

.SUFFIXES:

export CC

CUTEST_PATH := $(abspath $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

CUTEST_RUN=$(CUTEST_PATH)/cutest_run
CUTEST_MOCK=$(CUTEST_PATH)/cutest_mock
CUTEST_PROX=$(CUTEST_PATH)/cutest_prox
CUTEST_WORK=$(CUTEST_PATH)/cutest_work

ifeq ($(MAKECMDGOALS),sanitize)
	CC=clang
endif

ifneq (${Q},@)
	V=-v
else
	V=-n
endif

CUTEST_SRC_DIR ?=./
CUTEST_TEST_DIR ?=./

CUTEST_SRC_DIR:=$(abspath $(CUTEST_SRC_DIR))
CUTEST_TEST_DIR:=$(abspath $(CUTEST_TEST_DIR))
CUTEST_TMP_PATH:=$(abspath $(CUTEST_SRC_DIR))/.cutest

PEDANTIC:=-pedantic -Wall
# Some nice flags for compiling cutest-tests with good quality
ifneq ($(findstring clang,$(CC)),clang)
	CUTEST_CFLAGS?=-g
else
	CUTEST_CFLAGS?=
endif

ifneq (${LENIENT},0)
	ifeq ("$(STD)","-std=c11")
		CUTEST_CFLAGS+=-D"CUTEST_LENIENT_ASSERTS=1"
	endif
endif

export ASAN_OPTIONS=strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1
export UBSAN_OPTIONS=print_stacktrace=1

#CUTEST_CFLAGS+=-Wno-pragmas

ifeq ($(findstring gcc,$(CC)),gcc)
	CUTEST_CFLAGS+=-D"CUTEST_GCC=1"
endif
ifeq ($(findstring clang,$(CC)),clang)
	CUTEST_CFLAGS+=-D"CUTEST_CLANG=1"
endif

ifeq ($(MAKECMDGOALS),sanitize)
	CUTEST_CFLAGS+=-fsanitize=address,leak,undefined -fno-omit-frame-pointer
	ASAN_OPTIONS=detect_leaks=1
	export ASAN_OPTIONS
endif

info:
	$(Q)echo "CUTEST_SRC_DIR: $(CUTEST_SRC_DIR)" && \
	echo "CUTEST_TEST_DIR: $(CUTEST_TEST_DIR)" && \
	echo "CUTEST_PATH: $(CUTEST_PATH)"

.cutest:
	$(Q)mkdir -p $@

.cutest/cutest_make: $(CUTEST_PATH)/cutest_make.c $(CUTEST_PATH)/cutest_make.h .cutest
	$(Q)$(CC) $< -pedantic -Wall -O2 -std=c11 -flto -o $@ -g -D'CUTEST_PATH="$(CUTEST_PATH)/"' -D'CUTEST_SRC_PATH="$(CUTEST_SRC_DIR)/"' -D'CUTEST_TST_PATH="$(CUTEST_TEST_DIR)/"' -D'CUTEST_INC_PATH="$(CUTEST_PATH)/"' -D'CUTEST_TMP_PATH="$(CUTEST_TMP_PATH)/"'

.cutest/cutest: .cutest/cutest_make
	$(Q)$< $@

.cutest/cutest_work: .cutest/cutest_make
	$(Q)$< $@

check: .cutest/cutest .cutest/cutest_make .cutest/cutest_work
	$(Q)$^

clean::
	$(Q).cutest/cutest_make clean && $(RM) *.xml
