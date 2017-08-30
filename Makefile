#
# Makefile to build releases of CUTest
#

#export COVERAGE=1
VERSION=$(shell grep 'CUTEST_VERSION' src/cutest.h | cut -d'"' -f2)
DATE=$(shell /bin/date +"%Y-%m-%d")

all:
	@echo "======================================" && \
	echo "Running regression tests with valgrind" && \
	echo "======================================" && \
	echo "" && \
	$(MAKE) -r --no-print-directory regression_tests && \
	$(MAKE) -r --no-print-directory README.rst && \
	$(MAKE) -r --no-print-directory clean && \
	$(MAKE) -r --no-print-directory release

regression_tests:
	@echo "examples:" && \
	$(MAKE) -r --no-print-directory -C examples valgrind >/dev/null && echo "OK" && \
	echo "my_project_with_a_test_folder_inside_the_src_folder:" && \
	$(MAKE) -r --no-print-directory -C examples/complex_directory_structure/my_project_with_a_test_folder_inside_the_src_folder/src valgrind >/dev/null && echo "OK" && \
	echo "my_project_with_separate_src_and_test_folders:" && \
	$(MAKE) -r --no-print-directory -C examples/complex_directory_structure/my_project_with_separate_src_and_test_folders valgrind >/dev/null && echo "OK" && \
	echo ""

release: cutest-$(VERSION).tar.gz

cutest-$(VERSION).tar.gz:
	@echo "==============================================" && \
	echo "Checking git repository for version tag v$(VERSION)" && \
	echo "==============================================" && \
	echo "" && \
	git tag -l | grep "v$(VERSION)" && echo "Version v$(VERSION) is already released" && \
	git tag -l | grep "v$(VERSION)" && exit 1 || \
	echo "OK" && \
	echo "" && \
	echo "=================" && \
	echo "Creating tar-ball" && \
	echo "=================" && \
	echo "" && \
	mkdir cutest-$(VERSION) && \
	cp -r LICENSE examples src cutest-$(VERSION)/. && \
	rm -rf cutest-$(VERSION)/.git* && \
	rm -rf cutest-$(VERSION)/Makefile && \
	tar -c cutest-$(VERSION) | gzip > $@ && \
        rm -rf cutest-$(VERSION) && \
	echo "OK" && \
	echo "" && \
	tar -xzf cutest-$(VERSION).tar.gz && cp Makefile cutest-$(VERSION)/. && \
	echo "==============================" && \
	echo "Testing tar-ball with valgrind" && \
	echo "==============================" && \
	echo "" && \
	$(MAKE) -r --no-print-directory -C cutest-$(VERSION) regression_tests && \
	rm -rf cutest-$(VERSION) && \
	grep 'yyyy-mm-dd' README.rst >/dev/null && (bash -c 'echo -e "\e[31mWARNING!!!\e[0m: cutest.h still as yyyy-mm-dd. You need to update it."'; echo "Removing $@"; rm -f $@; true); \
	grep ' * * v$(VERSION) $(DATE)' README.rst >/dev/null || (bash -c 'echo -e "\e[31mWARNING!!!\e[0m: cutest.h has wrong date."'; echo "Removing $@"; rm -f $@: true); \
	git status | grep 'git add' >/dev/null && (git status; echo "Removing $@" && rm -f $@; true); \
	test -f $@ && echo "\nALL OK!!! Go ahead and upload the cutest-$(VERSION).tar.gz then inform the public!\n"

foo:
	bash -c 'echo -e "\e[31mWARNING\e[0m: cutest.h still as yyyy-mm-dd. You need to update it."'

examples/cutest_help.rst:
	@$(MAKE) -r --no-print-directory -C examples cutest_help.rst

README.rst: examples/cutest_help.rst
	@echo "=============================="; \
	echo "Generating new README.rst file"; \
	echo "=============================="; \
	echo "" && \
	mv $^ $@ && echo "OK" && \
	$(MAKE) -r --no-print-directory -C examples clean && \
	echo ""

clean:
	@rm -rf tmp cutest-* *~ && \
	$(MAKE) -r --no-print-directory -C examples clean && \
	$(MAKE) -r --no-print-directory -C examples/complex_directory_structure/my_project_with_a_test_folder_inside_the_src_folder/src clean && \
	$(MAKE) -r --no-print-directory -C examples/complex_directory_structure/my_project_with_separate_src_and_test_folders clean
