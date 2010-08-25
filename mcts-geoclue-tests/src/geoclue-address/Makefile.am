testdir = $(TEST_BIN_DIR)
test_PROGRAMS = \
        test_geoclue_address \
        test_geoclue_address_async \
	test_geoclue_address_details 

UTILS_FILES =                \
        ../utils/test_geoclue_utils.h  \
        ../utils/test_geoclue_utils.c

test_geoclue_address_SOURCES = \
		$(UTILS_FILES) \
		test_geoclue_address.c 

test_geoclue_address_async_SOURCES = \
		$(UTILS_FILES) \
		test_geoclue_address_async.c 

test_geoclue_address_details_SOURCES = \
		$(UTILS_FILES) \
                test_geoclue_address_details.c


AM_CFLAGS = \
	$(GEOCLUE_CFLAGS)  \
	-I../utils

LDADD = $(GEOCLUE_LIBS)
 
