# Reuse the Q variable for making cutest test running verbose too.
Q ?=@

CUTEST_PATH := $(subst /cutest.mk,,$(abspath $(lastword $(MAKEFILE_LIST))))

ifneq (${Q},@)
	V=-v
else
	V=-n
endif

CUTEST_SRC_DIR ?=./

# Some nice flags for compiling cutest-tests with good quality
CUTEST_CFLAGS+=-g -pedantic -Wall -Wextra -std=c11

cutest_info:
	echo $(CUTEST_PATH)

cutest_turead: $(CUTEST_PATH)/cutest_turead.c
	$(Q)$(CC) $< $(CUTEST_CFLAGS) -I$(CUTEST_PATH) -o $@

# Generate a very strange C-program including cutest.h for int main().
cutest_run.c: $(CUTEST_PATH)/cutest.h Makefile
	$(Q)echo "#include \"cutest.h\"" > $@

# Build a tool to generate a test suite runner.
cutest_run: cutest_run.c
	$(Q)$(CC) $< $(CUTEST_CFLAGS) -I$(CUTEST_PATH) -DCUTEST_RUN_MAIN -o $@

# Generate a very strange C-program including cutest.h for int main().
cutest_mock.c: $(CUTEST_PATH)/cutest.h Makefile
	$(Q)echo "#include \"cutest.h\"" > $@

# Build a tool to generate a test runner program.
cutest_mock: cutest_mock.c
	$(Q)which cproto >/dev/null || \
	(echo "ERROR: cproto is not installed in your path"; false) && \
	$(CC) $< $(CUTEST_CFLAGS) -I$(CUTEST_PATH) -DCUTEST_MOCK_MAIN -o $@

# Generate a very strange C-program including cutest.h for int main().
cutest_prox.c: $(CUTEST_PATH)/cutest.h Makefile
	$(Q)echo "#include \"cutest.h\"" > $@

# Build a tool to generate a test suite runner.
cutest_prox: cutest_prox.c
	$(Q)$(CC) $< $(CUTEST_CFLAGS) -I$(CUTEST_PATH) -DCUTEST_PROX_MAIN -o $@

# Extract all functions called by the design under test.
%.tu: $(subst _test,,%_test.c)
	$(Q)g++ -fdump-translation-unit -c $< && \
	cat $<.*.tu | c++filt > $@ && \
	rm -f $<.*.tu

.PRECIOUS: %_mockables.o
%_mockables.o: $(CUTEST_SRC_DIR)/%.c
	$(Q)$(CC) -o $@ -c $< $(CFLAGS)

.PRECIOUS: %_mockables.lst
%_mockables.lst: %_mockables.o
	$(Q)nm $< | sed 's/.* //g' | grep -v '__stack_' | sort -u > $@

.PRECIOUS: %_mockables.s
%_mockables.s: $(CUTEST_SRC_DIR)/%.c
	$(Q)$(CC) -S -fverbose-asm -fvisibility=hidden -fno-inline -g -O0 \
	-o $@ -c $^ $(CUTEST_CFLAGS) -D"static=" -D"inline=" -D"main=MAIN"

.PRECIOUS: %_proxified.s
%_proxified.s: %_mockables.s %_mockables.lst cutest_prox
	$(Q)./cutest_prox $< $(subst .s,.lst,$<) > $@

.PRECIOUS: %_mocks.h
# Generate mocks from the call()-macro in a source-file.
%_mocks.h: $(CUTEST_SRC_DIR)/%.c %_mockables.lst $(CUTEST_PATH)/cutest.h cutest_mock
	$(Q)./cutest_mock $< $(subst .c,_mockables.lst,$<) $(CUTEST_PATH) \
	$(CUTEST_IFLAGS) > $@; \

.PRECIOUS: %_test_run.c
# Generate a test-runner program code from a test-source-file
%_test_run.c: %_test.c %_mocks.h $(CUTEST_PATH)/cutest.h cutest_run
	$(Q)./cutest_run $(filter-out cutest.h,$^) > $@

# Compile a test-runner from the generate test-runner program code
%_test: %_proxified.s %_test_run.c
	$(Q)$(CC) -o $@ $^ $(CUTEST_CFLAGS) -I$(CUTEST_PATH) $(CUTEST_IFLAGS) \
	-DNDEBUG

# Print the CUTest manual
cutest_help.rst: $(CUTEST_PATH)/cutest.h
	$(Q)grep -e '^ * ' $< | \
	grep -v '*/' | \
	grep -v -e '^  ' | \
	sed -e 's/^ \* //g;s/^ \*//g' > $@

# Output the CUTest manual as HTML
cutest_help.html: cutest_help.rst
	$(Q)rst2html $< > $@

# Show the CUTest manual
cutest_help: cutest_help.rst
	$(Q)less $<

%_test.memcheck: %_test
	$(Q)valgrind -q --xml=yes --xml-file=$@ ./$< > /dev/null

check:: $(subst .c,,$(wildcard *_test.c))
	@R=true; \
	processors=`cat /proc/cpuinfo | grep processor | wc -l`; \
	for i in $^; do \
	  while [ `ps xa | grep -v grep | grep '_test ' | wc -l` -gt $$processors ]; do sleep 0.1; done; \
	  ./$$i $V -j || rm $$i || R=false & \
	done; \
	while [ `ps xa | grep -v grep | grep '_test ' | wc -l` -gt 0 ]; do \
	  sleep 1; \
	done; \
	echo "	"; `$$R`

memcheck:: $(subst .c,.memcheck,$(wildcard *_test.c))

valgrind:: $(subst .c,,$(wildcard *_test.c))
	@R=true; \
	processors=`cat /proc/cpuinfo | grep processor | wc -l`; \
	for i in $^; do \
	  while [ `ps xa | grep -v grep | grep '_test ' | wc -l` -gt $$processors ]; do sleep 0.1; done; \
	  valgrind -q ./$$i -v -j || rm $$i || R=false & \
	done; \
	while [ `ps xa | grep -v grep | grep '_test ' | wc -l` -gt 0 ]; do \
	  sleep 1; \
	done; \
	`$$R`

clean::
	$(Q)$(RM) -f cutest_* \
	*_test_run.c \
	*_mocks.h \
	*.junit_report.xml \
	*.memcheck \
	*_test \
	*.tu \
	*.gcda \
	*.gcno \
	*_mockables.* \
	*_proxified.* \
	cutest_help.rst \
	cutest_help.html \
	*~
