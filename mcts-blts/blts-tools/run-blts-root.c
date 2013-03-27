/* ------------------------------------------------------------------------- *
 * Copyright (C) 2013 Jolla Ltd.
 * Contact: Matti Kosola <matti.kosola@jollamobile.com>
 * License: GPLv2
 * ------------------------------------------------------------------------- */

#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	if ( argc <= 1 ) {
		printf("Enter command to parameter e.g: /usr/sbin/run-blts-root ls -al\n");
		exit(EXIT_FAILURE);
	}

	if ( setresgid(0, 0, 0) == -1 ) {
		perror("setresgid");
		exit(EXIT_FAILURE);
	}

	if ( setresuid(0, 0, 0) == -1 ) {
                perror("setresuid");
                exit(EXIT_FAILURE);
        }

	++argv, --argc;
	execvp(*argv, argv);
	perror(*argv);
	exit(EXIT_FAILURE);
}
