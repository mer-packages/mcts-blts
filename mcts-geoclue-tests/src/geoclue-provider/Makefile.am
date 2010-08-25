testdir = $(TEST_BIN_DIR)
test_PROGRAMS = \
        test_geoclue_provider 

UTILS_FILES =                \
        ../utils/test_geoclue_utils.h  \
        ../utils/test_geoclue_utils.c

test_geoclue_provider_SOURCES = \
		$(UTILS_FILES) \
		test_geoclue_provider.c 

AM_CFLAGS = \
	$(GEOCLUE_CFLAGS)  \
	-I../utils

LDADD = $(GEOCLUE_LIBS)
 
