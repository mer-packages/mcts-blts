/* blts_dep_check.c -- Rule-based dependency checker

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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/utsname.h>
#include <glob.h>

#include "blts_log.h"

/** helper to sort out what to log */
#define llog(level, severity, s...)					\
	{ if((level)>=(severity)) BLTS_DEBUG(s); }

/** find first line in file matching '.*str.*' , return it (free() yourself) or null */
static char* grep_line(FILE* f, char* str)
{
	char* found = 0;
	char *buf = 0;
	size_t buf_sz = 0;
	ssize_t read_len = 0;

	while((read_len = getline(&buf,&buf_sz,f)) != -1)
	{
		if(strstr(buf,str))
		{
			if(asprintf(&found,"%s",buf) < 0)
			{
				BLTS_LOGGED_PERROR("malloc");
				return 0;
			}
			break;
		}
	}
	return found;
}


static int check_file(char* name, int loglevel)
{
	llog(loglevel,2,"Checking file \"%s\": ",name);

	errno=0;
	if(!access(name,R_OK))
	{
		llog(loglevel,2,"ok.\n");
		return 0;
	}
	/* nok */
	llog(loglevel,1,"File check failed for %s - %s\n",name,strerror(errno));
	return -errno;
}


static int check_kmod(char* name, int loglevel)
{
	struct utsname uname_buf;

	llog(loglevel,2,"Checking kernel module \"%s\": ",name);


	/* #1: module needs to have an entry in /lib/modules/`uname -r`/modules.dep */

	uname(&uname_buf);

	char* moddep=0;
	if(asprintf(&moddep,"/lib/modules/%s/modules.dep",uname_buf.release) < 0)
	{
		BLTS_LOGGED_PERROR("malloc");
		return -1;
	}

	FILE *deps = 0;
	char* mod_line=0;
	char* mod_dep_path=0;
	char* mod_full_path=0;
	int retval = 0;

	errno=0;

	if(access(moddep,R_OK) != 0)
	{
		if(errno != ENOENT)
		{
			BLTS_LOGGED_PERROR("Cannot determine /lib/modules layout");
			retval=-1;
			goto cleanup;
		}
		/* Some systems still have an old-style layout for /lib/modules . */
		/* We'll continue with that assumption in this branch */
		if(asprintf(&mod_full_path,"/lib/modules/%s/%s.ko",uname_buf.release,name)<0)
		{
			BLTS_LOGGED_PERROR("malloc");
			retval=-1;
			goto cleanup;
		}
	}
	else
	{
		if(!(deps = fopen(moddep,"r")))
		{
			llog(loglevel,1,"Module check for %s failed - fopen(\"%s\") failed\n",name,moddep);
			retval = -1;
			goto cleanup;
		}
		mod_line = grep_line(deps,name);

		if(!mod_line)
		{
			llog(loglevel,1,"Module check for %s failed - module not in modules.dep\n",name);
			retval = -1;
			goto cleanup;
		}

		llog(loglevel,2,"found in modules.dep; ");

		/* #2: look up module according to deps info */

		/* try to figure out path to module */
		char *colon = strchr(mod_line, ':');
		if(!colon)
		{
			llog(loglevel,1,"Module check for %s failed - malformed line in modules.dep\n",name);
			retval=-1;
		}
		size_t path_len = colon-mod_line;
		mod_dep_path = malloc(path_len+1);
		strncpy(mod_dep_path,mod_line,path_len);
		mod_dep_path[path_len] = '\0';

		/* this depends on modutils version */
		if(strncmp(mod_line,"/lib/modules",12) != 0)
		{
			/* relative path */
			if(asprintf(&mod_full_path,"/lib/modules/%s/%s",uname_buf.release,mod_dep_path)<0)
			{
				BLTS_LOGGED_PERROR("malloc");
				retval=-1;
				goto cleanup;
			}
		}
		else
		{
			/* absolute path */
			if(asprintf(&mod_full_path,"%s",mod_dep_path)<0)
			{
				BLTS_LOGGED_PERROR("malloc");
				retval=-1;
				goto cleanup;
			}
		}
		llog(loglevel,2,"modules.dep ok.\n");
	}
	retval = check_file(mod_full_path,loglevel);

cleanup:
	if(deps) fclose(deps);
	if(moddep) free(moddep);
	if(mod_line) free(mod_line);
	if(mod_dep_path) free(mod_dep_path);
	if(mod_full_path) free(mod_full_path);
	return retval;
}

static int check_lib(char* name, int loglevel)
{
	llog(loglevel,2,"Checking library \"%s\": ",name);

	/* Maybe check LD_LIBRARY_PATH? */
	char *valid_dirs[] = {
		"/lib",	"/usr/lib", 0
	};

	char* dirname=0;
	char* fullpath_glob=0;
	int found=0;
	glob_t gbuf;

	for(int i=0; valid_dirs[i]; ++i)
	{
		dirname=valid_dirs[i];
		if(asprintf(&fullpath_glob,"%s/%s*",dirname,name)<0)
		{
			BLTS_LOGGED_PERROR("malloc");
			return -1;
		}
		if(!glob(fullpath_glob,0,0,&gbuf))
		{
			for(unsigned int j=0;j<gbuf.gl_pathc;++j) llog(loglevel,2,"found %s; ",gbuf.gl_pathv[j]);
			found += gbuf.gl_pathc;
		}
		globfree(&gbuf);

		free(fullpath_glob); fullpath_glob=0;
	}

	if(found>0)
	{
		llog(loglevel,2,"ok.\n");
	}
	else
	{
		llog(loglevel,1,"Library check for %s failed - no matching libraries found\n",name);
	}
	return (found>0)?0:-1;

}

static int check_bin(char* name, int loglevel)
{
	llog(loglevel,2,"Checking executable \"%s\": ",name);

	/* We don't trust PATH */
	char *valid_exe_dirs[] = {
		"", "/bin", "/sbin", "/usr/bin", "/usr/sbin", "/usr/X11/bin", 0
		/* /usr/local/...: FHS says this is empty after OS install */
	};

	char* dirname=0;
	char* fullpath=0;
	int found=0;
	for(int i=0; valid_exe_dirs[i]; ++i)
	{
		if (valid_exe_dirs[0] != '\0' && *name != '/')
                {
			dirname=valid_exe_dirs[i];
			if(asprintf(&fullpath,"%s/%s",dirname,name)<0)
			{
				BLTS_LOGGED_PERROR("malloc");
				return -1;
			}
                }
                else {
			if(asprintf(&fullpath, "%s", name) < 0)
			{
				BLTS_LOGGED_PERROR("malloc");
				return -1;
			}
                }
		if(!access(fullpath,X_OK))
		{
			llog(loglevel,2,"found '%s'; ",fullpath);
			found++;
		}

		free(fullpath); fullpath=0;
	}

	if(found>0)
	{
		llog(loglevel,2,"ok.\n");
	}
	else
	{
		llog(loglevel,1,"Binary check for %s failed - no matching binaries found\n",name);
	}
	return (found>0)?0:-1;
}

/**
 * Parse rulesfile and dispatch checks for each line
 * @param rulesfile File with rules for check.
 * @param loglevel 0=quiet 1=errors 2=all
 * @return 0 = all checks ok
 *
 * ( todo: might be prettier do do this with yacc&bison )
 */
int depcheck(char *rulesfile, int loglevel)
{
	FILE *rfile = 0;
	char *linebuf = 0;
	size_t buf_sz = 0;
	ssize_t read_len = 0;

	if(!(rfile = fopen(rulesfile,"r")))
	{
		llog(loglevel,1,"%s - fopen(\"%s\") failed\n",__FUNCTION__,rulesfile);
		return -1;
	}

	int retval = 0;
	const char delims[]=" \t";
	char *work_s = 0, *work_start=0;
	char *rule_kind = 0;
	char *rule_type = 0;
	char *rule_arg = 0;
	char* arg_tail = 0;
	int check_res;

	int lineno = 0;
	while((read_len = getline(&linebuf, &buf_sz, rfile)) != -1)
	{
		lineno++;
		check_res = 0;
		if(work_start)
		{
			free(work_start);
			work_start = 0;
		}

		if(asprintf(&work_start,"%s",linebuf)<0)
		{
			BLTS_LOGGED_PERROR("malloc");
			break;
		}
		work_s = work_start;

		if(work_s[0] == '#' || work_s[0] == '\n') continue; /* comment / empty line->skip*/

		/* tailing \n */
		if((arg_tail=strchr(work_s,'\n')))
		{
			arg_tail[0]='\0';
		}

		rule_kind = strsep(&work_s, delims);
		rule_type = strsep(&work_s, delims);
		rule_arg = work_s;

		if(!(rule_kind && rule_type && rule_arg))
		{
			llog(loglevel,1,"%s - Malformed rule on line %d\n",__FUNCTION__,lineno);
			continue;
		}

		if(!strcmp(rule_type,"kmod"))
		{
			check_res=check_kmod(rule_arg,loglevel);
		}
		else if(!strcmp(rule_type,"bin"))
		{
			check_res=check_bin(rule_arg,loglevel);
		}
		else if(!strcmp(rule_type,"lib"))
		{
			check_res=check_lib(rule_arg,loglevel);
		}
		else if(!strcmp(rule_type,"file"))
		{
			check_res=check_file(rule_arg,loglevel);
		}
		else
		{
			llog(loglevel,2,"%s - Can't parse rule on line %d, skipping\n",__FUNCTION__,lineno);
			continue;
		}

		if(check_res)
		{
			if(!strcmp(rule_kind,"required"))
			{
				llog(loglevel,1,"Missing required %s \"%s\".\n",rule_type,rule_arg);
				retval=-1;
			}
			else if(!strcmp(rule_kind,"optional"))
			{
				llog(loglevel,2,"Missing optional %s \"%s\".\n",rule_type,rule_arg);
			}
		}
		else
		{
			llog(loglevel,2,"Found %s \"%s\".\n",rule_type,rule_arg);
		}
	}

	if(work_start) free(work_start);
	if(linebuf) free(linebuf);
	fclose(rfile);
	return retval;
}

