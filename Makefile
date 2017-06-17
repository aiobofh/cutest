#
# Makefile to build releases of CUTest
#

VERSION=$(shell grep 'CUTEST_VERSION' src/cutest.h | cut -d'"' -f2)

all:
	@echo "========================" && \
	echo "Running regression tests" && \
	echo "========================" && \
	make -s regression_tests && \
	make -s README.rst && \
	make -s clean && \
	make -s release

regression_tests:
	make -s -C examples && \
	make -s -C examples/complex_directory_structure/my_project_with_a_test_folder_inside_the_src_folder/src && \
	make -s -C examples/complex_directory_structure/my_project_with_separate_src_and_test_folders

release: cutest-$(VERSION).tar.gz

cutest-$(VERSION).tar.gz:
	@echo "Checking git repository for version tag v$(VERSION)" && \
	git tag -l | grep "v$(VERSION)" && echo "Version v$(VERSION) is already released" && \
	git tag -l | grep "v$(VERSION)" && exit 1 || \
	echo "OK, version not found... Creating release" && \
	mkdir cutest-$(VERSION) && \
	cp -r LICENSE examples src cutest-$(VERSION)/. && \
	rm -rf cutest-$(VERSION)/.git* && \
	rm -rf cutest-$(VERSION)/Makefile && \
	tar -c cutest-$(VERSION) | gzip > $@ && \
        rm -rf cutest-$(VERSION) && \
	mkdir tmp && \
	cp cutest-$(VERSION).tar.gz tmp/. && \
	cd tmp && \
	tar -xzf cutest-$(VERSION).tar.gz && \
	cd cutest-$(VERSION) && \
	cp ../../Makefile . && \
	echo "========================" && \
	echo "Testing tar-ball" && \
	echo "========================" && \
	make -s regression_tests && \
	cd ../../ && \
	rm -rf tmp && \
	echo "" && \
	echo "ALL OK!!! Go ahead and upload the cutest-$(VERSION).tar.gz then inform the public!" && \
	echo ""

README.rst:
	@cd examples && \
	make -s cutest_help.rst && \
	mv cutest_help.rst ../$@ && \
	make -s clean

clean:
	@rm -rf tmp cutest-* *~ && \
	make -s -C examples clean && \
	make -s -C examples/complex_directory_structure/my_project_with_a_test_folder_inside_the_src_folder/src clean && \
	make -s -C examples/complex_directory_structure/my_project_with_separate_src_and_test_folders clean
