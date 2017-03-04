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

# Extract all functions called by the design under test.
%.tu: $(subst _test,,%_test.c)
	$(Q)g++ -fdump-translation-unit -c $< -D"call(func)=func" && \
	cat $<.*.tu | c++filt > $@ && \
	rm -f $<.*.tu

%_mockables.o: $(CUTEST_SRC_DIR)/%.c
	$(Q)$(CC) -o $@ -c $< -O0 -D"call(func)=func"

.PRECIOUS: %_mocks.h
# Generate mocks from the call()-macro in a source-file.
%_mocks.h: $(CUTEST_SRC_DIR)/%.c %_mockables.o $(CUTEST_PATH)/cutest.h cutest_mock
	$(Q)./cutest_mock $< $(subst .c,_mockables.o,$<) $(CUTEST_PATH) $(CUTEST_IFLAGS) > $@; \

.PRECIOUS: %_test_run.c
# Generate a test-runner program code from a test-source-file
%_test_run.c: %_test.c %_mocks.h $(CUTEST_PATH)/cutest.h cutest_run
	$(Q)./cutest_run $(filter-out cutest.h,$^) > $@

# Compile a test-runner from the generate test-runner program code
%_test: %_test_run.c
	$(Q)$(CC) $^ $(CUTEST_CFLAGS) -I$(CUTEST_PATH) $(CUTEST_IFLAGS) -DNDEBUG -o $@

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

check:: $(subst .c,,$(wildcard *_test.c))
	@R=true; \
	processors=`cat /proc/cpuinfo | grep processor | wc -l`; \
	for i in $^; do \
	  while [ `ps xa | grep -v grep | grep _test | wc -l` -gt $$processors ]; do echo "wait"; sleep 0.1; done; \
	  ./$$i $V -j || rm $$i || R=false & \
	done; \
	while [ `ps xa | grep -v grep | grep '_test' | wc -l` -gt 0 ]; do \
	  echo "waiting..."; \
	  sleep 1; \
	done; \
	echo "	"; `$$R`

clean::
	$(Q)$(RM) -f cutest_* \
	*_test_run.c \
	*_mocks.h \
	*.junit_report.xml \
	*_test \
	*.tu \
	*.gcda \
	*.gcno \
	*_mockables.o \
	cutest_help.rst \
	cutest_help.html \
	*~
