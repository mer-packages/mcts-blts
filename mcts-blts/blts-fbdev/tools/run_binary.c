/* ------------------------------------------------------------------------- *
 * Copyright (C) 2013 Jolla Ltd.
 * Contact: Matti Kosola <matti.kosola@jollamobile.com>
 * License: GPLv2
 * ------------------------------------------------------------------------- */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char **argv)
{
	if ( argc <= 1 ) {
		printf("Enter command to parameter e.g: ./run-binary ls -al\n");
		exit(EXIT_FAILURE);
	}
	++argv, --argc;
	execvp(*argv, argv);
	perror(*argv);
	exit(EXIT_FAILURE);
}
