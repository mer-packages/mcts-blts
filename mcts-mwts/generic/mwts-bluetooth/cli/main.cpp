/* main.cpp -- Implementation of command line interface for mwts-bluetooth.
 *
 * This file is part of MCTS
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Tommi Toropainen; tommi.toropainen@nokia.com;
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */
#include <getopt.h>
#include "stable.h"
#include "BluetoothTest.h"

/*
    The usage of our test cases without MIN testframework is
    not supported. This application is for quick tests only
    and behaviour is not guaranteed.
*/

int main( int argc, char** argv )
{
    printf("main, do new\n");
    BluetoothTest* btTest = new BluetoothTest();
    printf("main, done new\n");


    printf("do btTest->Initialize()\n");
    btTest->Initialize();

    int c;
    char host[21];
    long int time=-1L;
    long int bytes=10485760L;
    int buffsize=672;
    BluetoothSocket::SocketRole role = BluetoothSocket::SocketRoleUndefined;
    // scan == 0, transfer == 1, pair == 2
    int type = -1;
    // connect == 0, disconnect == 1, remove == 2
    int dev = -1;
    bool isHeadset = false;

    while (1)
    {
        static struct option long_options[] =
        {
            {"scan",     no_argument,       0, 'a'},
            {"server",   no_argument, 	   0, 's'},
            {"client",   no_argument, 	   0, 'c'},
            {"headset",  no_argument, 	   0, 'e'},
            {"connect",  no_argument, 	   0, 'o'},
            {"disconnect",  no_argument,    0, 'd'},
            {"remove",   no_argument, 	   0, 'r'},
            {"host",     required_argument, 0, 'h'},
            {"time",     required_argument, 0, 't'},
            {"bytes",    required_argument, 0, 'b'},
            {"buffer",   required_argument, 0, 'f'},
            {"help",     required_argument, 0, 'p'},
            {0, 0, 0, 0}
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "a:s:c:h:t:b:f:e:o:d:r:p",
                         long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {
        case 0:
            /* If this option set a flag, do nothing else now. */
            if (long_options[option_index].flag != 0)
                break;
            printf ("option %s", long_options[option_index].name);
            if (optarg)
                printf (" with arg %s", optarg);
            printf ("\n");
            break;

        case 'p':
            printf("Help:\n\n");
            printf("[options]\n\n");
            printf("\tscan,         \tno_argument       \n");
            printf("\tserver,       \tno_argument       \n");
            printf("\tclient,       \tno_argument       \n");
            printf("\theadset,      \tno_argument       \n");
            printf("\tconnect,      \tno_argument       \n");
            printf("\tdisconnect,   \tno_argument       \n");
            printf("\tremove,       \tno_argument       \n");
            printf("\thost,         \thost mac address  \n");
            printf("\ttime,         \ttime of transfer  \n");
            printf("\tbytes,        \tbytes to transfer \n");
            printf("\tbuffer,       \tbuffer size       \n");
            printf("\thelp,         \thelp              \n");
            printf("\n\n\n[example]\n\n");
            printf("\tmwts-bluetooth-cli --connect --headset --host 00:00:00:00:00\n\n\n");
            break;

        case 'a':
            printf ("option -a\n");
            type = 0;
            break;

        case 'h':
            printf ("option -h with value `%s'\n", optarg);
            strcpy(host,optarg);
            break;

        case 't':
            printf ("option -t with value `%s'\n", optarg);
            sscanf(optarg, "%ld", &time);
            break;

        case 'b':
            printf ("option -b with value `%s'\n", optarg);
            sscanf(optarg, "%ld", &bytes);
            break;

        case 'f':
            printf ("option -f with value `%s'\n", optarg);
            sscanf(optarg, "%ld", &buffsize);
            break;

        case 'c':
            printf ("option -c with value `%s'\n", optarg);
            role = BluetoothSocket::SocketRoleClient;
            type = 1;
            break;
     
        case 's':
            printf ("option -s with value `%s'\n", optarg);
            role = BluetoothSocket::SocketRoleServer;
            type = 1;
            break;

        case 'e':
            printf ("option -e with value `%s'\n", optarg);
            isHeadset = true;
            break;

        case 'o':
            printf ("option -o with value `%s'\n", optarg);
            type = 2;
            dev = 0;
            break;

        case 'd':
            printf ("option -d with value `%s'\n", optarg);
            type = 2;
            dev = 1;
            break;

        case 'r':
            printf ("option -d with value `%s'\n", optarg);
            type = 2;
            dev = 2;
            break;
     
        case '?':
            /* getopt_long already printed an error message. */
            break;
     
        default:
            break;
        }
    }

    switch(type)
    {
    case 0:
        printf("do btTest->Scan()\n"),
        btTest->Scan();
        break;
    case 1:
        printf("do btTest->Transfer()\n");
        btTest->Transfer(role,BluetoothSocket::SocketTypeL2CAP,host,bytes,buffsize,time);
        break;
    case 2:
        printf("do Device command");
        btTest->Device(dev,host,0,true,isHeadset);
        break;
    default:
        printf("No command specified");
        break;
    }

    printf("do btTest->Uninitialize()\n");
    btTest->Uninitialize();

    delete btTest;
    printf("end\n");
    return 0;
}
