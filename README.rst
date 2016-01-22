
CUTest - The C Unit Test framework
==================================

Author: AiO (AiO Secure Teletronics)
Project site: https://github.com/aiobofh/cutest

Thank you for downloading the CUTest framework! I hope it will make your
software development, using test-driven design an easier task.

CUTest is a C testing framework written in pure C. The idea behind
CUTest is to provide a platform independent C Unit Test framework,
but I guess it will only work in Linux for GCC anyway :). It's the
thought that counts. Please join me and port it to other enviroments.

The CUTest framework is tightly bound to a very specific build system
layout too. So let's admit that GNU Make is also needed.

Features
--------

* Automated C-function controllable mocks (with some code footprint
  in the form of the inclusion of call.h and usage of the call() macro)
* C-Function stubbing
* Generic asserts
* JUnit XML reports for Jenkins integration
* Very few dependencies to other tools (`echo`, `gcc`, `make` and `cproto`)
* In-line documentation to ReSTructured Text or HTML
  (requires additional tools: `grep`, `sed` and `rst2html`)

In-line documentation to ReSTructured Text and/or HTML
------------------------------------------------------

You can always read the cutest.h file, since it's the only one around :D.

... or you can generate a ReSTructured Text-style output using by adding
another target to your Makefile::

  # Print the CUTest manual
  cutest_help.rst: cutest.h
      $(Q)grep -e '^ * ' cutest.h | \
      grep -v '/' | \
      grep -v -e '^  ' | \
      sed -e 's/^ \* //g;s/^ \*+//g;s/ \*$//g' > $@

... or if you prefer HTML you can add this too::

  # Print the CUTest manuial as HTML
  cutest_help.html: cutest_help.rst
      rst2html $< > $@

Flat directory ''structure''
----------------------------

The CUTest framework expects you to work with your design and tests in
the same folder, however this is not mandatory. You can tweak the example
Makefile snippets here to change that behaviour.

However, this is the tested way to structure your design under test and
your test suites::

  src/cutest.h
  src/call.h
  :
  src/dut.c       <- your program (design under test) (can #include call.h)
  src/dut_test.c  <- test suite for dut.c (should #include cutest.h)
  src/Makefile

... So keep your clean:-target clean ;).

Example
-------

foo_test.c::

  #include "cutest.h"

  test(adder_shall_add_two_arguments_and_return_their_sum) {
    assert_eq(3, adder(1, 2), "adder shall return 3 = 1 + 2");
    assert_eq(6, adder(3, 3), "adder shall return 6 = 3 + 3");
  }

  test(foo_shall_call_the_adder_function_once_with_correct_arguments) {
    // When calling foo() the call(adder(i, j))-macro will call a mock.
    foo(1, 2);
    assert_eq(1, cutest_mock.adder.call_count, "adder shall be called once");
    assert_eq(1, cutest_mock.adder.args.arg0, "first argument shall be 1");
    assert_eq(2, cutest_mock.adder.args.arg1, "second argument shall be 2");
  }

  test(foo_shall_return_the_adder_functions_result_unmodified) {
    cutest_mock.adder.retval = 123456;
    assert_eq(123456, foo(1, 2), "foo shall return adder's return value");
  }

foo.c::

  #include "call.h" // Contains a book-mark macro to find mockables.

  int adder(int a, int b) { return a + b; }
  int foo(int i, int j) { return call(adder(a, b)); }

Makefile::

  .PRECIOUS: %_mocks.h
  # Generate mocks from the call()-macro in a source-file.
  %_mocks.h: %.c cutest.h cutest_mock
        $(Q)./cutest_mock $< > $@

  .PRECIOUS: %_test_run.c
  # Generate a test-runner program code from a test-source-file
  %_test_run.c: %_test.c %_mocks.h cutest.h cutest_run
        $(Q)./cutest_run $(filter-out cutest.h,$^) > $@

  # Compile a test-runner from the generate test-runner program code
  %_test: %_test_run.c
        $(Q)$(CC) $^ -Wall -pedantic -std=c11 -o $@

  check: $(subst .c,,$(wildcard *_test.c))
        $(Q)R=true; \
        for i in $<; do \
           ./$$i -v -j || (rm $$i || R=false); \
        done; `$$R

Command line::

  $ make foo_test
  $ ./foo_test
  ...

It's a lot of rules and targets for one simple test case, but it scales
very well for rerunning only needed tests.

To compile the test runner you should never ever have `CUTEST_RUN_MAIN`
nor `CUTEST_MOCK_MAIN` defined to the compiler. They are used to compile
the *CUTest test runner generator* and the *CUTest mock generator*
respectively.

The call() macro
----------------

By default this is defined in the call.h header. This must be used to
call _any_ function that is to be possible to replace with a mock using
the CUTest-framework.

However when writing tests, all call() macros will actually call the
corresponding mock-up function instead of the real deal. Hence the call()
macro is overreidden by cutest.h inclusion.

The test() macro
----------------

Every test is defined with this macro.

Example::

  test(main_should_return_0_on_successful_execution)
  {
    ... Test body ...
  }

The assert_eq() macro
---------------------

This macro makes it easy to understand the test-case flow.

Example::

  ...
  assert_eq(1, 1, "1 should be eqial to 1");
  ...

Test initialization
-------------------

In between every test() macro the CUTest framework will clear all the
mock controls and test framwork state so that every test is run in
isolation.

Test execution
--------------

When executing tests the elapsed time for execution is sampled and used
in the JUnit report. Depending on command line options an output is printed
to the console, either as a short version with '.' for successful test run
and 'F' for failed test run, but if set to verbose '-v' '[PASS]' and
'[FAIL]' output is produced. What triggers a failure is if an assert_eq()
is not fulfilled.

If the test runner is started with verbose mode '-v' the offending assert
will be printed to the console directly after the fail. If in normal mode
all assert-failures will be collected and printed in the shutdown process.

Shutdown process
----------------

At the end of the execution the CUTest test-runner program will output
a JUnit XML report if specified with the -j command line option.


CUTest mock generator
=====================

This is a tool that can be used to generate mock-up functions. It
inspects a specified source-code file (written i C language) and looks
for uses of the cutest-specific call() macro which should encapsulate
every function that is replaceable with a mock when developing code using
test-driven design.

Requirements
------------

To be able to generate well formatted function declarations to mutate
into mock-ups this tool make use of the ``cproto`` tool.

How to compile the tool
-----------------------

Makefile::

  # Generate a very strange C-program including cutest.h for int main().
  cutest_mock.c: cutest.h
      echo "#include \"cutest.h\"" > $@

  # Build a tool to generate a test runner program.
  cutest_mock: cutest_mock.c
      $(Q)$(CC) $< $(CUTEST_CFLAGS) -DCUTEST_MOCK_MAIN -o $@

Usage
-----

If you *need* to run the tool manually this is how::

  $ ./cutest_mock design_under_test.c

And it will scan the source-code for uses of the `call()` macro and
output a header file-style text, containing everything needed to test
your code alongside with the `cutest.h` file.

However, if you use the Makefile targets specified in the beginning of
this document you will probably not need to run it manually.

Mock-ups
--------

The cutest_mock tool scans the design under test for call() macros, and
create a mock-up control stucture, unique for every callable mockable
function, so that tests can be fully controlled.

The control structures are encapsulated in the global struct instance
called 'mocks'.

In a test they can be accessed like this::

  mocks.<name_of_called_function>.<property>...

If you have::

  FILE* fp = call(fopen("filename.c", "r"));

in your code, a mock called cutest_mock_fopen() will be generated. It
will affect the cutest_mock.fopen mock-up control structure.

For accurate information please build your <dut>_mocks.h file and
inspect the structs yourself.

Stubbing
--------

To stub a function encapsulated in a call() macro in your design under
test you can easily write your own stub in your test-file, just pointing
the cutest_mock.<dut>.func function pointer to your stub.


CUTest test runner generator
============================

The cutest_run tool will parse your test suite and produce an executable
program with some command line options to enable you to control it a
little bit.

How to build the tool
---------------------

Makefile::

  # Generate a very strange C-program including cutest.h for int main().
  cutest_run.c: cutest.h Makefile
    $(Q)echo "#include \"cutest.h\"" > $@

  # Build a tool to generate a test suite runner.
  cutest_run: cutest_run.c
    $(Q)$(CC) $< $(CUTEST_CFLAGS) -DCUTEST_RUN_MAIN -o $@

Usage
-----

If you *need* to run the tool manually this is how::

  $ ./cutest_run dut_test.c dut_mocks.h

And it will scan the test suite source-code for uses of the `test()`
macro and output a C program containing everything needed to test
your code alongside with the `cutest.h` file.

However, if you use the Makefile targets specified in the beginning of
this document you will probably not need to run it manually.

The test runner program
-----------------------

The generated test runner program will inventory all the tests in the
specified suite and run them in the order that they appear in the suite.

The first thing that happens is the Startup process, then all tests are
run in isolation, followed by the Shutdown process.
