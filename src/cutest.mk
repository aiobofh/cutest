# Reuse the Q variable for making cutest test running verbose too.
Q ?=@

CUTEST_PATH := $(subst /cutest.mk,,$(abspath $(lastword $(MAKEFILE_LIST))))

ifneq (${Q},@)
	V=-v
else
	V=-n
endif

CUTEST_SRC_DIR ?=./
CUTEST_TEST_DIR ?=./

# Some nice flags for compiling cutest-tests with good quality
CUTEST_CFLAGS+=-g -pedantic -Wall -Wextra -std=c11

cutest_info:
	@echo "Test-folder: $(CUTEST_TEST_DIR)"
	@echo "CUTest-path: $(CUTEST_PATH)"

cutest_turead: $(CUTEST_PATH)/cutest_turead.c
	$(Q)$(CC) $< $(CUTEST_CFLAGS) -I$(CUTEST_PATH) -o $@

# Generate a very strange C-program including cutest.h for int main().
$(CUTEST_TEST_DIR)/cutest_run.c: $(CUTEST_PATH)/cutest.h Makefile
	$(Q)echo "#include \"cutest.h\"" > $@

# Build a tool to generate a test suite runner.
$(CUTEST_TEST_DIR)/cutest_run: $(CUTEST_TEST_DIR)/cutest_run.c
	$(Q)$(CC) $< $(CUTEST_CFLAGS) -I$(CUTEST_PATH) -DCUTEST_RUN_MAIN -o $@

# Generate a very strange C-program including cutest.h for int main().
$(CUTEST_TEST_DIR)/cutest_mock.c: $(CUTEST_PATH)/cutest.h Makefile
	$(Q)echo "#include \"cutest.h\"" > $@

# Build a tool to generate a test runner program.
$(CUTEST_TEST_DIR)/cutest_mock: $(CUTEST_TEST_DIR)/cutest_mock.c
	$(Q)which cproto >/dev/null || \
	(echo "ERROR: cproto is not installed in your path"; false) && \
	$(CC) $< $(CUTEST_CFLAGS) -I$(CUTEST_PATH) -DCUTEST_MOCK_MAIN -o $@

# Generate a very strange C-program including cutest.h for int main().
$(CUTEST_TEST_DIR)/cutest_prox.c: $(CUTEST_PATH)/cutest.h Makefile
	$(Q)echo "#include \"cutest.h\"" > $@

# Build a tool to generate a test suite runner.
$(CUTEST_TEST_DIR)/cutest_prox: $(CUTEST_TEST_DIR)/cutest_prox.c
	$(Q)$(CC) $< $(CUTEST_CFLAGS) -I$(CUTEST_PATH) -DCUTEST_PROX_MAIN -o $@

# Extract all functions called by the design under test.
$(CUTEST_TEST_DIR)/%.tu: $(subst _test,,$(CUTEST_TEST_DIR)/%_test.c)
	$(Q)g++ -fdump-translation-unit -c $< && \
	cat $<.*.tu | c++filt > $@ && \
	rm -f $<.*.tu

.PRECIOUS: $(CUTEST_TEST_DIR)/%_mockables.o
$(CUTEST_TEST_DIR)/%_mockables.o: $(CUTEST_SRC_DIR)/%.c
	$(Q)$(CC) -o $@ -c $< $(CFLAGS)

.PRECIOUS: $(CUTEST_TEST_DIR)/%_mockables.lst
$(CUTEST_TEST_DIR)/%_mockables.lst: $(CUTEST_TEST_DIR)/%_mockables.o
	$(Q)nm $< | sed 's/.* //g' | grep -v '__stack_' | sort -u > $@

.PRECIOUS: $(CUTEST_TEST_DIR)/%_mockables.s
$(CUTEST_TEST_DIR)/%_mockables.s: $(CUTEST_SRC_DIR)/%.c
	$(Q)$(CC) -S -fverbose-asm -fvisibility=hidden -fno-inline -g -O0 \
	-o $@ -c $^ $(CUTEST_CFLAGS) -D"static=" -D"inline=" -D"main=MAIN"

.PRECIOUS: $(CUTEST_TEST_DIR)/%_proxified.s
$(CUTEST_TEST_DIR)/%_proxified.s: $(CUTEST_TEST_DIR)/%_mockables.s $(CUTEST_TEST_DIR)/%_mockables.lst $(CUTEST_TEST_DIR)/cutest_prox
	$(Q)$(CUTEST_TEST_DIR)//cutest_prox $< $(subst .s,.lst,$<) > $@

.PRECIOUS: $(CUTEST_TEST_DIR)/%_mocks.h
# Generate mocks from the call()-macro in a source-file.
$(CUTEST_TEST_DIR)/%_mocks.h: $(CUTEST_SRC_DIR)/%.c $(CUTEST_TEST_DIR)/%_mockables.lst $(CUTEST_PATH)/cutest.h $(CUTEST_TEST_DIR)/cutest_mock
	$(Q)$(CUTEST_TEST_DIR)/cutest_mock $< $(addprefix $(CUTEST_TEST_DIR)/,$(notdir $(subst .c,_mockables.lst,$<))) $(CUTEST_PATH) \
	$(CUTEST_IFLAGS) > $@; \

.PRECIOUS: $(CUTEST_TEST_DIR)/%_test_run.c
# Generate a test-runner program code from a test-source-file
$(CUTEST_TEST_DIR)/%_test_run.c: $(CUTEST_TEST_DIR)/%_test.c $(CUTEST_TEST_DIR)/%_mocks.h $(CUTEST_PATH)/cutest.h $(CUTEST_TEST_DIR)/cutest_run
	$(Q)cd $(CUTEST_TEST_DIR) && ./cutest_run $(filter-out cutest.h,$(notdir $^)) > $(notdir $@)

# Compile a test-runner from the generate test-runner program code
$(CUTEST_TEST_DIR)/%_test: $(CUTEST_TEST_DIR)/%_proxified.s $(CUTEST_TEST_DIR)/%_test_run.c
	$(Q)$(CC) -o $@ $^ $(CUTEST_CFLAGS) -I$(CUTEST_PATH) -I$(abspath $(CUTEST_TEST_DIR)) -I$(abspath $(CUTEST_SRC_DIR)) $(CUTEST_IFLAGS) \
	-DNDEBUG

# Print the CUTest manual
$(CUTEST_TEST_DIR)/cutest_help.rst: $(CUTEST_PATH)/cutest.h
	$(Q)grep -e '^ * ' $< | \
	grep -v '*/' | \
	grep -v -e '^  ' | \
	sed -e 's/^ \* //g;s/^ \*//g' > $@

# Output the CUTest manual as HTML
$(CUTEST_TEST_DIR)/cutest_help.html: $(CUTEST_TEST_DIR)/cutest_help.rst
	$(Q)rst2html $< > $@

# Show the CUTest manual
cutest_help: $(CUTEST_TEST_DIR)/cutest_help.rst
	$(Q)less $<

$(CUTEST_TEST_DIR)/%_test.memcheck: $(CUTEST_TEST_DIR)/%_test
	$(Q)valgrind -q --xml=yes --xml-file=$@ ./$< > /dev/null

check:: $(subst .c,,$(wildcard $(CUTEST_TEST_DIR)/*_test.c))
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

memcheck:: $(subst .c,.memcheck,$(wildcard $(CUTEST_TEST_DIR)/*_test.c))

valgrind:: $(subst .c,,$(wildcard $(CUTEST_TEST_DIR)/*_test.c))
	echo $<
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
	$(Q)$(RM) -f $(CUTEST_TEST_DIR)/cutest_* \
	$(CUTEST_TEST_DIR)/*_test_run.c \
	$(CUTEST_TEST_DIR)/*_mocks.h \
	$(CUTEST_SRC_DIR)/*.junit_report.xml \
	$(CUTEST_TEST_DIR)/*.memcheck \
	$(CUTEST_TEST_DIR)/*_test \
	$(CUTEST_TEST_DIR)/*.tu \
	$(CUTEST_TEST_DIR)/*_mockables.* \
	$(CUTEST_TEST_DIR)/*_proxified.* \
	$(CUTEST_TEST_DIR)/cutest_help.rst \
	$(CUTEST_TEST_DIR)/cutest_help.html \
	$(CUTEST_TEST_DIR)/*~
