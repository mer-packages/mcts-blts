testdir = $(TEST_BIN_DIR)
test_PROGRAMS = \
        test_geoclue_position \
        test_geoclue_position_async 

UTILS_FILES =                \
        ../utils/test_geoclue_utils.h  \
        ../utils/test_geoclue_utils.c


test_geoclue_position_SOURCES = \
		$(UTILS_FILES) \
		test_geoclue_position.c 

test_geoclue_position_async_SOURCES = \
		$(UTILS_FILES) \
		test_geoclue_position_async.c 

AM_CFLAGS = \
	$(GEOCLUE_CFLAGS)  \
	-I../utils

LDADD = $(GEOCLUE_LIBS)
 


