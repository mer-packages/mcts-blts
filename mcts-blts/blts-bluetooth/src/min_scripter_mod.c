/* min_scripter_mod.c -- Min scripter plugin interface for BT tests

   Copyright (C) 2000-2010, Nokia Corporation.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#define _GNU_SOURCE

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <test_module_api.h>
#include <min_parser.h>
#include <min_logger.h>
#include <min_test_event_if.h>
#include <min_ipc_mechanism.h>
#include <blts_log.h>


#include "bt_fute.h"

/* Min calls getters for these variables (see below): */
char *module_date = __DATE__;
char *module_time = __TIME__;
TTestModuleType module_type     = ETestClass;
unsigned int    module_version  = 200940;


/* Required definitions, for Min scripter interface. From Min template. */
/* snip >>>-------------------------------------------------------------*/

#ifdef ENTRYTC
#undef ENTRYTC
#endif
#define ENTRYTC(_l_, _n_,_f_)                                           \
        do {                                                            \
        TestCaseInfoTC* tc = (TestCaseInfoTC*)malloc(sizeof(TestCaseInfoTC));\
        if( tc == NULL ) break;                                         \
        STRCPY(tc->name_,_n_,MaxTestCaseName);                          \
        tc->test_ = _f_;                                                \
        tc->id_   = dl_list_size(_l_)+1;                                \
        dl_list_add( _l_, (void*)tc );                                  \
        } while(0)

typedef struct _TestCaseInfoTC TestCaseInfoTC;
typedef struct _ScriptVariable ScriptVariable;

typedef int (*ptr2testtc)( MinItemParser * tcr );

struct _TestCaseInfoTC
{
        char name_[MaxTestCaseName];
        ptr2testtc test_;
        unsigned int id_;
};

struct _ScriptVariable
{
        char *var_name_;
        TSBool is_initialized_;
        char *var_value_;
};

/* <<< snip ------------------------------------------------------------*/

/* Forward declarations */
int Scan(MinItemParser *args);
int L2CAP_EchoServer(MinItemParser *args);
int L2CAP_EchoClient(MinItemParser *args);
int RFCOMM_EchoServer(MinItemParser *args);
int RFCOMM_EchoClient(MinItemParser *args);
int HCI_Connect(MinItemParser *args);
int HCI_Audit_Incoming_Connect(MinItemParser *args);
int DriverCheck(MinItemParser *args);
int HCI_Transfer_ACL_data(MinItemParser *args);
int HCI_Receive_ACL_data(MinItemParser *args);
int HCI_Change_Name(MinItemParser *args);
int HCI_Verify_Name(MinItemParser *args);
int HCI_Change_Class(MinItemParser *args);
int HCI_Verify_Class(MinItemParser *args);
int HCI_Reset_Connection(MinItemParser *args);
int redirect_stdout_stderr(char* target);


/* Required functions for a Min scripter plugin. --->>> */

/* Min calls this when it needs a list of test cases implemented by us. */
int ts_get_test_cases(DLList ** list)
{
	ENTRYTC(*list, "Scan", Scan);
	ENTRYTC(*list, "DriverCheck", DriverCheck);
	ENTRYTC(*list, "L2CAP_EchoServer", L2CAP_EchoServer);
	ENTRYTC(*list, "L2CAP_EchoClient", L2CAP_EchoClient);
	ENTRYTC(*list, "RFCOMM_EchoServer", RFCOMM_EchoServer);
	ENTRYTC(*list, "RFCOMM_EchoClient", RFCOMM_EchoClient);
	ENTRYTC(*list, "HCI_Connect", HCI_Connect);
	ENTRYTC(*list, "HCI_Audit_Incoming_Connect", HCI_Audit_Incoming_Connect);
	ENTRYTC(*list, "HCI_Transfer_ACL_data", HCI_Transfer_ACL_data);
	ENTRYTC(*list, "HCI_Receive_ACL_data", HCI_Receive_ACL_data);
	ENTRYTC(*list, "HCI_Change_Name", HCI_Change_Name);
	ENTRYTC(*list, "HCI_Verify_Name", HCI_Verify_Name);
	ENTRYTC(*list, "HCI_Change_Class", HCI_Change_Class);
	ENTRYTC(*list, "HCI_Verify_Class", HCI_Verify_Class);
	ENTRYTC(*list, "HCI_Reset_Connection", HCI_Reset_Connection);
	return 0;
}



/* Getters for module id data */
unsigned int get_module_type() { return module_type; }
unsigned int get_module_version() { return module_version; }
char* get_module_date() { return module_date; }
char* get_module_time() { return module_time; }

/* Comparison helper from Min template. */
int _look4callname (const void *a, const void *b)
{
        TestCaseInfoTC *tci = (TestCaseInfoTC *) a;
        return strcmp (tci->name_, (char *)b);
}

/* Min TC runner (from template, shm var passing removed) */
int ts_run_method (MinItemParser * item)
{
        DLList         *l;
        DLListIterator  it;
        int             retval;
        char           *callname;

        l = dl_list_create ();
        callname = INITPTR;

        retval = mip_get_next_string (item, &callname);
        if (retval != ENOERR) {
                retval = -1;
                goto EXIT;
        }

        ts_get_test_cases (&l);
        it = dl_list_find (dl_list_head (l)
                           , dl_list_tail (l)
                           , _look4callname, callname);

        if (it == DLListNULLIterator) {
                retval = -1;
                DELETE (callname);
                it = dl_list_head (l);
                while (it != DLListNULLIterator) {
                        free (dl_list_data (it));
                        dl_list_remove_it (it);
                        it = dl_list_head (l);
                }
                dl_list_free (&l);
                goto EXIT;
        }

        retval = ((TestCaseInfoTC *) dl_list_data (it))->test_ (item);

        DELETE (callname);
        it = dl_list_head (l);
        while (it != DLListNULLIterator) {
                free (dl_list_data (it));
                dl_list_remove_it (it);
                it = dl_list_head (l);
        }
        dl_list_free (&l);
EXIT:
        return retval;
}



/* These are scripter-callable functions for individual components. >>> */

/* BT Scan
 * args: logfile
 */
int Scan(MinItemParser *args)
{
	char *logfile = 0;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;

	log_open(logfile,0);
	redirect_stdout_stderr(logfile);

	int retval = fute_bt_scan();

	log_close();
	if(logfile) free(logfile);

	return retval;

parse_error:

	tm_printf(1, "", "Parse error in %s",__FUNCTION__);

	if(logfile) free(logfile);
	return EINVAL;
}


/* run L2CAP one-shot echo server
 * args: logfile
 */
int L2CAP_EchoServer(MinItemParser *args)
{
	char *logfile = 0;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;

	log_open(logfile,0);
	redirect_stdout_stderr(logfile);

	int retval = fute_bt_l2cap_echo_server();

	log_close();
	if(logfile) free(logfile);

	return retval;

parse_error:

	tm_printf(1, "", "Parse error in %s",__FUNCTION__);

	if(logfile) free(logfile);
	return EINVAL;
}

/* run L2CAP client echo test
 * args: logfile server_btaddr do_transfer_test(1|0)
 */
int L2CAP_EchoClient(MinItemParser *args)
{
	char *logfile = 0, *server_btaddr = 0;
	int want_transfer_test = -1;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(mip_get_next_string(args, &server_btaddr) < 0) goto parse_error;
	if(mip_get_next_int(args, &want_transfer_test) < 0) goto parse_error;

	log_open(logfile,0);
	redirect_stdout_stderr(logfile);

	int retval = fute_bt_l2cap_echo_client(server_btaddr, want_transfer_test);

	log_close();
	if(logfile) free(logfile);
	if(server_btaddr) free(server_btaddr);
	return retval;

parse_error:

	tm_printf(1, "", "Parse error in %s",__FUNCTION__);

	if(logfile) free(logfile);
	if(server_btaddr) free(server_btaddr);
	return EINVAL;
}


/* run RFCOMM one-shot echo server
 * args: logfile
 */
int RFCOMM_EchoServer(MinItemParser *args)
{
	char *logfile = 0;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;

	log_open(logfile,0);
	redirect_stdout_stderr(logfile);

	int retval = fute_bt_rfcomm_echo_server();

	log_close();
	if(logfile) free(logfile);

	return retval;

parse_error:

	tm_printf(1, "", "Parse error in %s",__FUNCTION__);

	if(logfile) free(logfile);
	return EINVAL;
}

/* run RFCOMM client echo test
 * args: logfile server_btaddr do_transfer_test(1|0)
 */
int RFCOMM_EchoClient(MinItemParser *args)
{
	char *logfile = 0, *server_btaddr = 0;
	int want_transfer_test = -1;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(mip_get_next_string(args, &server_btaddr) < 0) goto parse_error;
	if(mip_get_next_int(args, &want_transfer_test) < 0) goto parse_error;

	log_open(logfile,0);
	redirect_stdout_stderr(logfile);

	int retval = fute_bt_rfcomm_echo_client(server_btaddr, want_transfer_test);

	log_close();
	if(logfile) free(logfile);
	if(server_btaddr) free(server_btaddr);
	return retval;

parse_error:

	tm_printf(1, "", "Parse error in %s",__FUNCTION__);

	if(logfile) free(logfile);
	if(server_btaddr) free(server_btaddr);
	return EINVAL;
}

/* run HCI connect test
 * args: logfile remote_btaddr
 */
int HCI_Connect(MinItemParser *args)
{
	char *logfile = 0, *remote_btaddr = 0;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(mip_get_next_string(args, &remote_btaddr) < 0) goto parse_error;

	log_open(logfile,0);
	redirect_stdout_stderr(logfile);

	int retval = fute_bt_hci_connect_disconnect(remote_btaddr);

	log_close();
	if(logfile) free(logfile);
	if(remote_btaddr) free(remote_btaddr);

	return retval;

parse_error:

	tm_printf(1, "", "Parse error in %s",__FUNCTION__);

	if(logfile) free(logfile);
	if(remote_btaddr) free(remote_btaddr);
	return EINVAL;
}

/*
 * run HCI audit incoming connect test
 * args: logfile
 */
int HCI_Audit_Incoming_Connect(MinItemParser *args)
{
	char *logfile = 0;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;

	log_open(logfile,0);
	redirect_stdout_stderr(logfile);

	int retval = fute_bt_hci_audit_incoming_connect();

	log_close();
	if(logfile) free(logfile);

	return retval;

parse_error:

	tm_printf(1, "", "Parse error in %s",__FUNCTION__);

	if(logfile) free(logfile);
	return EINVAL;
}

/* run HCI transfer ACL data test
 * args: logfile remote_btaddr
 */
int HCI_Transfer_ACL_data(MinItemParser *args)
{
	char *logfile = 0, *remote_btaddr = 0;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(mip_get_next_string(args, &remote_btaddr) < 0) goto parse_error;

	log_open(logfile,0);
	redirect_stdout_stderr(logfile);

	int retval = fute_bt_hci_transfer_acl_data(remote_btaddr);

	log_close();
	if(logfile) free(logfile);
	if(remote_btaddr) free(remote_btaddr);

	return retval;

parse_error:

	tm_printf(1, "", "Parse error in %s",__FUNCTION__);

	if(logfile) free(logfile);
	if(remote_btaddr) free(remote_btaddr);
	return EINVAL;
}

int HCI_Change_Name(MinItemParser *args)
{
	char *logfile = 0;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;

	log_open(logfile,0);
	redirect_stdout_stderr(logfile);

	int retval = fute_bt_hci_change_name();

	log_close();
	if(logfile) free(logfile);

	return retval;

parse_error:

	tm_printf(1, "", "Parse error in %s",__FUNCTION__);

	if(logfile) free(logfile);
	return EINVAL;
}

int HCI_Verify_Name(MinItemParser *args)
{
	char *logfile = 0, *remote_btaddr = 0;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(mip_get_next_string(args, &remote_btaddr) < 0) goto parse_error;

	log_open(logfile,0);
	redirect_stdout_stderr(logfile);

	int retval = fute_bt_hci_verify_name(remote_btaddr);

	log_close();
	if(logfile) free(logfile);
	if(remote_btaddr) free(remote_btaddr);
	return retval;

parse_error:

	tm_printf(1, "", "Parse error in %s",__FUNCTION__);

	if(logfile) free(logfile);
	if(remote_btaddr) free(remote_btaddr);
	return EINVAL;
}

int HCI_Change_Class(MinItemParser *args)
{
	char *logfile = 0;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;

	log_open(logfile,0);
	redirect_stdout_stderr(logfile);

	int retval = fute_bt_hci_change_class();

	log_close();
	if(logfile) free(logfile);

	return retval;

parse_error:

	tm_printf(1, "", "Parse error in %s",__FUNCTION__);

	if(logfile) free(logfile);
	return EINVAL;
}
int HCI_Verify_Class(MinItemParser *args)
{
	char *logfile = 0, *remote_btaddr = 0;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(mip_get_next_string(args, &remote_btaddr) < 0) goto parse_error;

	log_open(logfile,0);
	redirect_stdout_stderr(logfile);

	int retval = fute_bt_hci_verify_class(remote_btaddr);

	log_close();
	if(logfile) free(logfile);
	if(remote_btaddr) free(remote_btaddr);

	return retval;

parse_error:

	tm_printf(1, "", "Parse error in %s",__FUNCTION__);

	if(logfile) free(logfile);
	if(remote_btaddr) free(remote_btaddr);
	return EINVAL;

}

/* run HCI reset connection test
 * args: logfile remote_btaddr
 */
int HCI_Reset_Connection(MinItemParser *args)
{
	char *logfile = 0, *remote_btaddr = 0;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;
	if(mip_get_next_string(args, &remote_btaddr) < 0) goto parse_error;

	log_open(logfile,0);
	redirect_stdout_stderr(logfile);

	int retval = fute_bt_hci_reset_connection(remote_btaddr);

	log_close();
	if(logfile) free(logfile);

	return retval;

parse_error:

	tm_printf(1, "", "Parse error in %s",__FUNCTION__);

	if(logfile) free(logfile);
	if(remote_btaddr) free(remote_btaddr);
	return EINVAL;
}
/* run HCI receive ACL data test
 * args: logfile
 */
int HCI_Receive_ACL_data(MinItemParser *args)
{
	char *logfile = 0;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;

	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;

	log_open(logfile,0);
	redirect_stdout_stderr(logfile);

	int retval = fute_bt_hci_receive_acl_data();

	log_close();
	if(logfile) free(logfile);

	return retval;

parse_error:

	tm_printf(1, "", "Parse error in %s",__FUNCTION__);

	if(logfile) free(logfile);
	return EINVAL;
}

int DriverCheck(MinItemParser *args)
{
	char *logfile = 0;

	if(mip_set_parsing_type(args, EQuoteStyleParsing) < 0) goto parse_error;
	if(mip_get_next_string(args, &logfile) < 0) goto parse_error;

	log_open(logfile,0);
	redirect_stdout_stderr(logfile);

	int retval = fute_bt_drivers_depcheck();

	log_close();
	if(logfile) free(logfile);

	return retval;

parse_error:

	tm_printf(1, "", "Parse error in %s",__FUNCTION__);

	if(logfile) free(logfile);
	return EINVAL;
}


int redirect_stdout_stderr(char* target)
{
	char* name = 0;

	if(asprintf(&name,"%s.stderr",target)<0) return -1;
	FILE *f = freopen(name,"a",stderr);
	free(name); name = 0;

	if(!f) return -1;
	fprintf(f,"--MARK--\n");

	if(asprintf(&name,"%s.stdout",target)<0) return -1;
	f = freopen(name,"a",stdout);
	free(name); name = 0;

	if(!f) return -1;

	fprintf(f,"--MARK--\n");
	return 0;
}


