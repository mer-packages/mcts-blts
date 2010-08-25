/*
# Copyright (C) 2010 Intel Corporation
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
# Authors:
#       Chen, Hao  <hao.h.chen@intel.com>
#
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pulse/pulseaudio.h>


enum PA_ACTION_TYPE
{
    PA_ACTON_NONE,
    PA_QUERY_SERVER_INFO,
    PA_QUERY_MEMORY_USAGE,
    PA_QUERY_SINKS_LIST,
    PA_QUERY_SOURCES_LIST,
    PA_QUERY_SINK_INPUTS,
    PA_QUERY_SOURCE_OUTPUTS,
    PA_QUERY_MODULES_LIST,
    PA_QUERY_AUTOLOAD_ENTRIES,
    PA_QUERY_CLIENTS,
    PA_CONTROL_SET_SINK_VOL,
    PA_CONTROL_SET_SOURCE_VOL,
    PA_CONTROL_SET_SINK_MUTE,
    PA_CONTROL_SET_SOURCE_MUTE,
    PA_CONTROL_SINK_INPUT_VOL,
    PA_CONTROL_SINK_INPUT_MUTE,
    PA_CONTROL_KILL_SINK_INPUT,
    PA_CONTROL_KILL_SOURCE_OUTPUT,
    PA_CONTROL_LOAD_MODULE,
    PA_CONTROL_UNLOAD_MODULE,
    PA_CONTROL_ADD_AUTOLOAD_SINK,
    PA_CONTROL_REMOVE_AUTOLOAD_SINK,
    PA_CONTROL_ADD_AUTOLOAD_SOURCE,
    PA_CONTROL_REMOVE_AUTOLOAD_SOURCE,
    PA_CONTROL_KILL_CLIENT,
    PA_CONTROL_SET_DEFAULT_SINK,
    PA_CONTROL_SET_DEFAULT_SOURCE,
    PA_CONTROL_SUSPEND_SINK,
    PA_CONTROL_SUSPEND_SOURCE,
    PA_CONTROL_MOVE_SINK_INPUT,
    PA_CONTROL_MOVE_SOURCE_OUTPUT,
};

typedef enum PA_ACTION_TYPE pa_action_t;
pa_action_t action;

/*for load/unload module*/
#define MODULE_ARGV_SEPERATOR '!'
char *module_name, *module_argv;
uint32_t idx = 0, idx2 = 0;
char *sink_name, *source_name;
unsigned channels;
pa_volume_t volume_value;
int mute;
int suspend;

/*PA API variables*/
static pa_context *context = NULL;
static pa_mainloop_api *mainloop_api = NULL;

/* A shortcut for terminating the application */
    void
quit (int ret)
{
    assert (mainloop_api);
    mainloop_api->quit (mainloop_api, ret);
}

    void
module_info_callback (struct pa_context *c,
	const struct pa_module_info *i, int eol, void *userdata)
{
    if (eol > 0)
    {
	/*end of list */
	quit (0);
    }
    else
    {
	/*display each module loaded */
	printf ("index=%u,name=%s,argv=%s,auto_unload=%i\n",
		i->index, i->name, i->argument, i->auto_unload);
    }

}

    void
sink_info_callback (pa_context * c,
	const pa_sink_info * i, int eol, void *userdata)
{
    uint8_t count;

    if (eol > 0)
    {
	/*end of list */
	quit (0);
    }
    else
    {
	pa_cvolume volume = i->volume;

	printf ("index=%u,name=%s,owner module index=%u,volume=<",
		i->index, i->name, i->owner_module);

	for (count = 0; count < (volume.channels); count++)
	{
	    printf ("%i: %u ", count, volume.values[count]);
	}

	printf (">,mute=%i,driver=%s,desc=%s\n",
		i->mute, i->driver, i->description);
    }
}

    void
source_info_callback (pa_context * c,
	const pa_source_info * i, int eol, void *userdata)
{
    uint8_t count;

    if (eol > 0)
    {
	/*end of list */
	quit (0);
    }
    else
    {
	pa_cvolume volume = i->volume;

	printf ("index=%u,name=%s,owner module index=%u,volume=<",
		i->index, i->name, i->owner_module);

	for (count = 0; count < (volume.channels); count++)
	{
	    printf ("%i: %u ", count, volume.values[count]);
	}

	printf (">,mute=%i,driver=%s,desc=%s\n",
		i->mute, i->driver, i->description);
    }
}

    void
autoload_info_callback (pa_context * c,
	const pa_autoload_info * i, int eol, void *userdata)
{
    if (eol > 0)
    {
	/*end of list */
	quit (0);
    }
    else
    {
	pa_autoload_type_t type = i->type;

	printf ("index=%u,name=%s,type=%s,module=%s, argv=%s\n",
		i->index, i->name,
		(type == PA_AUTOLOAD_SINK) ? "sink" : "source", i->module,
		i->argument);
    }
}

    void
context_index_callback (pa_context * c, uint32_t idx, void *userdata)
{
    printf ("index=%d\n", idx);
    if ((int) idx >= 0)
    {
	quit (0);
    }
    else
	quit (1);
}

    void
context_success_callback (pa_context * c, int success, void *userdata)
{
    if (success)
    {
	printf ("Succeed.\n");
	quit (0);
    }
    else
    {
	printf ("Failed.\n");
	quit (1);
    }
}

/*
   sampel format:
   PA_SAMPLE_U8 = 0,          < Unsigned 8 Bit PCM 
   PA_SAMPLE_ALAW,            < 8 Bit a-Law 
   PA_SAMPLE_ULAW,            < 8 Bit mu-Law 
   PA_SAMPLE_S16LE,           < Signed 16 Bit PCM, little endian (PC) 
   PA_SAMPLE_S16BE,           < Signed 16 Bit PCM, big endian 
   PA_SAMPLE_FLOAT32LE,       < 32 Bit IEEE floating point, little endian, range -1 to 1 
   PA_SAMPLE_FLOAT32BE,       < 32 Bit IEEE floating point, big endian, range -1 to 1 
   PA_SAMPLE_S32LE,           < Signed 32 Bit PCM, little endian (PC) 
   PA_SAMPLE_S32BE,           < Signed 32 Bit PCM, big endian (PC) 
   PA_SAMPLE_MAX,             < Upper limit of valid sample types 
   PA_SAMPLE_INVALID = -1     < An invalid value 
 */

    void
server_info_callback (pa_context * c, const pa_server_info * i,
	void *userdata)
{
    printf ("user name=%s,host name=%s,server version=%s,server name=%s,\
	    default sample type=%d %uch %uhz,default sink=%s,default source=%s\n", i->user_name, i->host_name, i->server_version, i->server_name, (i->sample_spec).format, (i->sample_spec).channels, (i->sample_spec).rate, i->default_sink_name, i->default_source_name);

    quit (0);
}

    void
stat_info_callback (pa_context * c, const pa_stat_info * i, void *userdata)
{
    printf ("\
	    Currently allocated memory blocks: %u\n\
	    Currentl total size of allocated memory blocks: %uB\n\
	    All allocated memory blocks during daemon time: %u\n\
	    Total size of all memory blocks allocated during memory lifetime: %uB\n\
	    Total size of sample cache: %uB\n", i->memblock_total, i->memblock_total_size, i->memblock_allocated, i->memblock_allocated_size, i->scache_size);

    quit (0);
}


    void
sink_input_info_callback (pa_context * c, const pa_sink_input_info * i,
	int eol, void *userdata)
{
    uint8_t count;

    if (eol > 0)
    {
	/*end of list */
	quit (0);
    }
    else
    {
	pa_cvolume volume = i->volume;

	/*only to print sink input brief info here */
	printf ("index=%d,name=%s,owner module idx=%u,client idx=%u,sink idx=%u,\
		volume=", i->index, i->name, i->owner_module, i->client,
		i->sink);

	for (count = 0; count < (volume.channels); count++)
	{
	    printf ("%i: %u ", count, volume.values[count]);
	}

	printf ("\n");
    }
}

    void
source_output_info_callback (pa_context * c, const pa_source_output_info * i,
	int eol, void *userdata)
{
    if (eol > 0)
    {
	/*end of list */
	quit (0);
    }
    else
    {
	/*only to print source output brief info here */
	printf
	    ("index=%d,name=%s,owner module idx=%u,client idx=%u,source idx=%u\n",
	     i->index, i->name, i->owner_module, i->client, i->source);
    }
}


    void
client_info_callback (pa_context * c, const pa_client_info * i,
	int eol, void *userdata)
{
    if (eol > 0)
    {
	/*end of list */
	quit (0);
    }
    else
    {
	printf ("index=%d,name=%s,owner module idx=%u,driver=%s\n",
		i->index, i->name, i->owner_module, i->driver);
    }
}

/* This is called whenever the context status changes */
    static void
context_state_callback (pa_context * c, void *userdata)
{

    pa_cvolume cv;

    assert (c);

    switch (pa_context_get_state (c))
    {
	case PA_CONTEXT_CONNECTING:
	case PA_CONTEXT_AUTHORIZING:
	case PA_CONTEXT_SETTING_NAME:
	    break;

	case PA_CONTEXT_READY:
	    {
		switch (action)
		{
		    /*query modules list */
		    case PA_QUERY_MODULES_LIST:
			pa_operation_unref (pa_context_get_module_info_list
				(context, module_info_callback, NULL));
			break;
			/*load module */
		    case PA_CONTROL_LOAD_MODULE:
			pa_operation_unref (pa_context_load_module
				(context, module_name, module_argv,
				 context_index_callback, NULL));
			break;
			/*unload module */
		    case PA_CONTROL_UNLOAD_MODULE:
			pa_operation_unref (pa_context_unload_module
				(context, idx, context_success_callback,
				 NULL));
			break;
			/*query sinks list */
		    case PA_QUERY_SINKS_LIST:
			pa_operation_unref (pa_context_get_sink_info_list
				(context, sink_info_callback, NULL));
			break;
			/*set default sink */
		    case PA_CONTROL_SET_DEFAULT_SINK:
			pa_operation_unref (pa_context_set_default_sink
				(context, sink_name, context_success_callback,
				 NULL));
			break;
			/*query source list */
		    case PA_QUERY_SOURCES_LIST:
			pa_operation_unref (pa_context_get_source_info_list
				(context, source_info_callback, NULL));
			break;
			/*set default source */
		    case PA_CONTROL_SET_DEFAULT_SOURCE:
			pa_operation_unref (pa_context_set_default_source
				(context, source_name,
				 context_success_callback, NULL));
			break;
			/*query server Info */
		    case PA_QUERY_SERVER_INFO:
			pa_operation_unref (pa_context_get_server_info
				(context, server_info_callback, NULL));
			break;
			/*query memory usage */
		    case PA_QUERY_MEMORY_USAGE:
			pa_operation_unref (pa_context_stat (context, stat_info_callback,
				    NULL));
			break;
			/*query sink inputs */
		    case PA_QUERY_SINK_INPUTS:
			pa_operation_unref (pa_context_get_sink_input_info_list
				(context, sink_input_info_callback, NULL));
			break;
			/*query source outputs */
		    case PA_QUERY_SOURCE_OUTPUTS:
			pa_operation_unref (pa_context_get_source_output_info_list
				(context, source_output_info_callback, NULL));
			break;
			/*query clients */
		    case PA_QUERY_CLIENTS:
			pa_operation_unref (pa_context_get_client_info_list
				(context, client_info_callback, NULL));
			break;
			/*set sink volume */
		    case PA_CONTROL_SET_SINK_VOL:
			pa_operation_unref (pa_context_set_sink_volume_by_name
				(context, sink_name,
				 pa_cvolume_set (&cv, channels, volume_value),
				 context_success_callback, NULL));
			break;
			/*set source volume */
		    case PA_CONTROL_SET_SOURCE_VOL:
			pa_operation_unref (pa_context_set_source_volume_by_name
				(context, source_name,
				 pa_cvolume_set (&cv, channels, volume_value),
				 context_success_callback, NULL));
			break;
			/*set sink input volume */
		    case PA_CONTROL_SINK_INPUT_VOL:
			pa_operation_unref (pa_context_set_sink_input_volume
				(context, idx,
				 pa_cvolume_set (&cv, channels, volume_value),
				 context_success_callback, NULL));
			break;

			/*mute/unmute */
		    case PA_CONTROL_SET_SINK_MUTE:
			pa_operation_unref (pa_context_set_sink_mute_by_name
				(context, sink_name, mute,
				 context_success_callback, NULL));
			break;
		    case PA_CONTROL_SET_SOURCE_MUTE:
			pa_operation_unref (pa_context_set_source_mute_by_name
				(context, source_name, mute,
				 context_success_callback, NULL));
			break;
		    case PA_CONTROL_SINK_INPUT_MUTE:
			pa_operation_unref (pa_context_set_sink_input_mute
				(context, idx, mute, context_success_callback,
				 NULL));
			break;
			/*kill */
		    case PA_CONTROL_KILL_SINK_INPUT:
			pa_operation_unref (pa_context_kill_sink_input
				(context, idx, context_success_callback,
				 NULL));
			break;
		    case PA_CONTROL_KILL_SOURCE_OUTPUT:
			pa_operation_unref (pa_context_kill_source_output
				(context, idx, context_success_callback,
				 NULL));
			break;
		    case PA_CONTROL_KILL_CLIENT:
			pa_operation_unref (pa_context_kill_client
				(context, idx, context_success_callback,
				 NULL));
			break;
			/*suspend */
		    case PA_CONTROL_SUSPEND_SINK:
			pa_operation_unref (pa_context_suspend_sink_by_index
				(context, idx, suspend,
				 context_success_callback, NULL));
			break;
		    case PA_CONTROL_SUSPEND_SOURCE:
			pa_operation_unref (pa_context_suspend_source_by_index
				(context, idx, suspend,
				 context_success_callback, NULL));
			break;
			/*move */
		    case PA_CONTROL_MOVE_SINK_INPUT:
			pa_operation_unref (pa_context_move_sink_input_by_index
				(context, idx, idx2, context_success_callback,
				 NULL));
			break;
		    case PA_CONTROL_MOVE_SOURCE_OUTPUT:
			pa_operation_unref (pa_context_move_source_output_by_index
				(context, idx, idx2, context_success_callback,
				 NULL));
			break;
		    default:
			break;

		}
		break;
	    }

	case PA_CONTEXT_TERMINATED:
	    quit (0);
	    break;

	case PA_CONTEXT_FAILED:
	default:
	    printf ("Connection failure.\n");
	    quit (1);
    }

    return;
}

    void
help_info ()
{
    printf ("\n\
	    Test PulseAudio Server Query and control API\n\
	    \n\
	    Querying\n\
	    - Server Infomation     : -qi\n\
	    - Memory usage          : -qu\n\
	    - Sinks list            : -qs\n\
	    - Sources list          : -qo\n\
	    - Sink inputs           : -qI\n\
	    - Source outputs        : -qO\n\
	    - Modules list          : -qm\n\
	    - Autoload entries      : -qa\n\
	    - Clients               : -qc\n\
	    \n\
	    Control\n\
	    - sinks volume                  : -cv  sink <sink name> <channel num> <value>\n\
	    - sources volume                : -cv  source <source name> <channel num> <value>\n\
	    - sink mute                     : -cu  sink <sink name>  [0|1]\n\
	    - source mute                   : -cu  source <source name> [0|1]\n\
	    - sink input volume             : -cv  input <input index>  <channel number> <value>\n\
	    - sink input mute               : -cu  input <input index> [0|1]\n\
	    - kill sink input               : -ck  input <sink input index>\n\
	    - kill source output            : -ck  output <source output index>\n\
	    - kill client                   : -ck  client <client index>\n\
	    - load module                   : -cm  load  <module name>  <argument> \n\
	    - unload module                 : -cm  unload  <index>\n\
	    - add/remove autoload entries   : -ca  addsink  <sink name> <module name> <module argv>\n\
	    : -ca  rmsink <index> \n\
	    : -ca  addsource  <source name> <module name> <module argv>\n\
	    : -ca  rmsource <index>\n\
	    - set default sink              : -cd  sink  <sink name>\n\
	    - set default source            : -cd  source  <source name>\n\
	    - suspend sink                  : -cp  sink <sink index> [0|1]\n\
	    - suspend source                : -cp  source <source index> [0|1]\n\
	    - move sink input               : -cc  input <input index> <sink index>\n\
	    - move source output            : -cc  output <output index> <source index>\n");
}

/*Query and control PulseAudio Server, via PA Server Query and control API

  Querying
  - Server Infomation  	: -qi
  - Memory usage			: -qu
  - Sinks list			: -qs
  - Sources list			: -qo
  - Sink inputs			: -qI
  - Source outputs		: -qO
  - Modules list			: -qm  
  - Autoload entries		: -qa
  - Clients				: -qc

  Control
  - sink volume/mute 			: -cv  sink <sink name> <channel num> <value> 
  - source volume/mute			: -cv  source <source name> <channel num>
  - sink mute 				: -cu  sink <sink name>  [0|1]
  - source mute				: -cu  source <source name> [0|1]
  - sink input volume 			: -cv  input <input index>  <channel number> <value>
  - sink input mute				: -cu  input <input index> [0|1]
  - kill sink input               : -ck  input <sink input index>
  - kill source output			: -ck  output <source output index>
  - kill client			        : -ck  client <client index>
  - load/unload module			: -cm  load  <module name>  <arguments, only
  support 1 for the moment> 
  : -cm  unload  <index>
  - add/remove autoload entries	: -ca  addsink  <sink name> <module name> <module argv>
  : -ca  rmsink <index> (could be name too,
  to do later)
  : -ca  addsource  <source name> <module name> <module argv>
  : -ca  rmsource <index> (could be name too,
  to do later)                                
  - set default sink              : -cd  sink  <sink name>
  - set default source            : -cd  source  <source name>
  - suspend sink					: -cp  sink <sink index> [0|1]
  - suspend source				: -cp  source <source index> [0|1]
  - move sink	input				: -cc  input <input index> <sink index>
  - move source output 			: -cc  output <output index> <source index>

  not to implement:
  - Sameples
 */

    int
main (int argc, char *argv[])
{
    int retval = 1;
    pa_mainloop *m = NULL;

    action = PA_ACTON_NONE;

    /*Parse cmd line paramters */
    while ((argc > 1) && (argv[1][0] == '-'))
    {
	switch (argv[1][1])
	{
	    case 'q':
		switch (argv[1][2])
		{
		    case 'm':
			action = PA_QUERY_MODULES_LIST;
			break;
		    case 's':
			action = PA_QUERY_SINKS_LIST;
			break;
		    case 'o':
			action = PA_QUERY_SOURCES_LIST;
			break;
		    case 'a':
			action = PA_QUERY_AUTOLOAD_ENTRIES;
			break;
		    case 'i':
			action = PA_QUERY_SERVER_INFO;
			break;
		    case 'u':
			action = PA_QUERY_MEMORY_USAGE;
			break;
		    case 'I':
			action = PA_QUERY_SINK_INPUTS;
			break;
		    case 'O':
			action = PA_QUERY_SOURCE_OUTPUTS;
			break;
		    case 'c':
			action = PA_QUERY_CLIENTS;
			break;
		    default:
			break;
		}
		break;
	    case 'c':
		switch (argv[1][2])
		{
		    /*module */
		    case 'm':
			if (!strcmp (argv[2], "load"))
			{
			    action = PA_CONTROL_LOAD_MODULE;
			    module_name = argv[3];
			    module_argv = argv[4];
			}
			else if (!strcmp (argv[2], "unload"))
			{
			    action = PA_CONTROL_UNLOAD_MODULE;
			    idx = atoi (argv[3]);
			}
			break;
			/*default sink/source */
		    case 'd':
			if (!strcmp (argv[2], "sink"))
			{
			    action = PA_CONTROL_SET_DEFAULT_SINK;
			    sink_name = argv[3];
			}
			else if (!strcmp (argv[2], "source"))
			{
			    action = PA_CONTROL_SET_DEFAULT_SOURCE;
			    source_name = argv[3];
			}
			break;
			/*autoload */
		    case 'a':
			if (!strcmp (argv[2], "addsink"))
			{
			    action = PA_CONTROL_ADD_AUTOLOAD_SINK;
			    sink_name = argv[3];
			    module_name = argv[4];
			    module_argv = argv[5];
			}
			else if (!strcmp (argv[2], "rmsink"))
			{
			    action = PA_CONTROL_REMOVE_AUTOLOAD_SINK;
			    idx = atoi (argv[3]);
			}
			else if (!strcmp (argv[2], "addsource"))
			{
			    action = PA_CONTROL_ADD_AUTOLOAD_SOURCE;
			    source_name = argv[3];
			    module_name = argv[4];
			    module_argv = argv[5];
			}
			else if (!strcmp (argv[2], "rmsource"))
			{
			    action = PA_CONTROL_REMOVE_AUTOLOAD_SOURCE;
			    idx = atoi (argv[3]);
			}
			break;
			/*set volume */
		    case 'v':
			if (!strcmp (argv[2], "sink"))
			{
			    action = PA_CONTROL_SET_SINK_VOL;
			    sink_name = argv[3];
			    channels = atoi (argv[4]);
			    volume_value = atoi (argv[5]);
			}
			else if (!strcmp (argv[2], "source"))
			{
			    action = PA_CONTROL_SET_SOURCE_VOL;
			    source_name = argv[3];
			    channels = atoi (argv[4]);
			    volume_value = atoi (argv[5]);
			}
			else if (!strcmp (argv[2], "input"))
			{
			    action = PA_CONTROL_SINK_INPUT_VOL;
			    idx = atoi (argv[3]);
			    channels = atoi (argv[4]);
			    volume_value = atoi (argv[5]);
			}
			break;
			/*mute/unmute */
		    case 'u':
			if (!strcmp (argv[2], "sink"))
			{
			    action = PA_CONTROL_SET_SINK_MUTE;
			    sink_name = argv[3];
			    mute = atoi (argv[4]);
			}
			else if (!strcmp (argv[2], "source"))
			{
			    action = PA_CONTROL_SET_SOURCE_MUTE;
			    source_name = argv[3];
			    mute = atoi (argv[4]);
			}
			else if (!strcmp (argv[2], "input"))
			{
			    action = PA_CONTROL_SINK_INPUT_MUTE;
			    idx = atoi (argv[3]);
			    mute = atoi (argv[4]);
			}
			break;
			/*kill */
		    case 'k':
			if (!strcmp (argv[2], "input"))
			{
			    action = PA_CONTROL_KILL_SINK_INPUT;
			    idx = atoi (argv[3]);
			}
			else if (!strcmp (argv[2], "output"))
			{
			    action = PA_CONTROL_KILL_SOURCE_OUTPUT;
			    idx = atoi (argv[3]);
			}
			else if (!strcmp (argv[2], "client"))
			{
			    action = PA_CONTROL_KILL_CLIENT;
			    idx = atoi (argv[3]);
			}
			break;
			/*suspend */
		    case 'p':
			if (!strcmp (argv[2], "sink"))
			{
			    action = PA_CONTROL_SUSPEND_SINK;
			    idx = atoi (argv[3]);
			    suspend = atoi (argv[4]);
			}
			else if (!strcmp (argv[2], "source"))
			{
			    action = PA_CONTROL_SUSPEND_SOURCE;
			    suspend = atoi (argv[4]);
			}
			break;
			/*move */
		    case 'c':
			if (!strcmp (argv[2], "input"))
			{
			    action = PA_CONTROL_MOVE_SINK_INPUT;
			    idx = atoi (argv[3]);	/*input idx */
			    idx2 = atoi (argv[4]);	/*sink idx */
			}
			else if (!strcmp (argv[2], "output"))
			{
			    action = PA_CONTROL_MOVE_SOURCE_OUTPUT;;
			    idx = atoi (argv[3]);	/*output idx */
			    idx2 = atoi (argv[4]);	/*source idx */
			}
			break;
		    default:
			break;
		}
		break;
	    default:
		break;
	}
	argv++;
	argc--;
    }

    if (PA_ACTON_NONE == action)
    {
	/*display help */
	help_info ();
	return 1;
    }

    /*debug */
#ifdef DEBUG
    printf("1=%s, 2=%s, 3=%s, 4=%s\n",argv[1],argv[2],argv[3],argv[4]);
    printf("action=%d, module_name=%s, module_argv=%s\n",action, module_name, module_argv);
#endif

    /* Set up a new main loop */
    if (!(m = pa_mainloop_new ()))
    {
	printf ("pa_mainloop_new() failed.\n");
	goto quit;
    }

    mainloop_api = pa_mainloop_get_api (m);

    /* Create a new connection context */
    if (!(context = pa_context_new (mainloop_api, "Moblin Test")))
    {
	printf ("pa_context_new() failed.\n");
	goto quit;
    }
    pa_context_set_state_callback (context, context_state_callback, NULL);

    /* Connect the context to default server */
    if (pa_context_connect (context, NULL, (pa_context_flags_t) 0, NULL) < 0)
    {
	printf ("pa_context_connect() failed.\n");
	goto quit;
    }

    /* Run the main loop (built-in, unlimited iterations, 
       until main loop's quit routine) */
    if (pa_mainloop_run (m, &retval) < 0)
    {
	printf ("pa_mainloop_run() failed.\n");
	goto quit;
    }

quit:

    /*Free connection context */
    if (context)
    {
	pa_context_unref (context);
	context = NULL;
    }

    /*Free main loop */
    if (m)
    {
	pa_mainloop_free (m);
    }

    return retval;
}
