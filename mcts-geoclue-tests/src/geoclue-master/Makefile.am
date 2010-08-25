testdir = $(TEST_BIN_DIR)
test_PROGRAMS = \
        test_geoclue_master \
	test_geoclue_master_client_position \
 	test_geoclue_master_client_position_async \
	test_geoclue_master_client_address \
 	test_geoclue_master_client_address_async \
	test_geoclue_master_client


UTILS_FILES =                \
        ../utils/test_geoclue_utils.h  \
        ../utils/test_geoclue_utils.c


test_geoclue_master_SOURCES = \
		$(UTILS_FILES) \
		test_geoclue_master.c 

test_geoclue_master_client_position_SOURCES = \
		$(UTILS_FILES) \
		test_geoclue_master_client_position.c 

test_geoclue_master_client_position_async_SOURCES = \
		$(UTILS_FILES) \
		test_geoclue_master_client_position_async.c 

test_geoclue_master_client_address_SOURCES = \
		$(UTILS_FILES) \
		test_geoclue_master_client_address.c 

test_geoclue_master_client_address_async_SOURCES = \
		$(UTILS_FILES) \
		test_geoclue_master_client_address_async.c 

test_geoclue_master_client_SOURCES = \
		$(UTILS_FILES) \
		test_geoclue_master_client.c 

AM_CFLAGS = \
	$(GEOCLUE_CFLAGS)  \
	-I../utils

LDADD = $(GEOCLUE_LIBS)
 


