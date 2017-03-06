CUTEST_CFLAGS+=-fprofile-arcs -ftest-coverage

coverage.xml: check
	$(Q)gcovr -r . -e '/usr.*' -e '.*_test.c' -e 'cutest.h' -e '.*_mocks.h' -e 'error.h' -e '.*_test_run.c' -x > $@

check::
	$(Q)echo "Lines covered:"; \
	gcovr -r . -e '/usr.*' -e '.*_test.c' -e '.*cutest.h' -e '.*_mocks.h' -e 'error.h' -e '.*_test_run.c' | egrep -v '^File' | egrep -v '^-' | egrep -v '^Directory' | grep -v 'GCC Code'; \
	echo ""; \
	echo "Branches covered:"; \
	gcovr -r . -e '/usr.*' -e '.*_test.c' -e '.*cutest.h'  -e '.*_mocks.h' -e 'error.h' -e '.*_test_run.c' -b | egrep -v '^File' | egrep -v '^-' | egrep -v '^Directory' | grep -v 'GCC Code';

clean::
	$(Q)rm -rf coverage.xml *.gcno *.gcda
