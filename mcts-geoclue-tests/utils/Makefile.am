testdir = $(TEST_BIN_DIR)
bin_SCRIPTS = uexec

script_in_file = geoclue_test.sh.in
test_SCRIPTS = $(script_in_file:.sh.in=.sh)
             

$(test_SCRIPTS): $(script_in_file) Makefile
	@sed -e "s|\@test_bin_dir\@|$(testdir)|" $< > $@
EXTRA_DIST =                    \
        $(script_in_file)     

DISTCLEANFILES = \
        $(script_in_file)                    

