/*
* Copyright (c) Bull S.A.  2007 All Rights Reserved.
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of version 2 of the GNU General Public License as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it would be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* Further, this software is distributed without any warranty that it is
* free of the rightful claim of any third person regarding infringement
* or the like.  Any license provided herein, whether implied or
* otherwise, applies only to this software file.  Patent licenses, if
* any, provided herein do not apply to combinations of this program with
* other software, or any other product whatsoever.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
* History:
* Created by: Cyril Lacabanne (Cyril.Lacabanne@bull.net)
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <rpc/rpc.h>
#include <sys/socket.h>
#include <utmp.h>
#include <sys/time.h>
#include <netdb.h>

//Standard define
#define PROCNUM 1
#define VERSNUM 1

//Other define
#define NBCASE 2

typedef struct {
	//List parameters here
	int bufsnd;
	int bufrec;
} params;

int main(int argn, char *argc[])
{
	//Program parameters : argc[1] : HostName or Host IP
	//                                         argc[2] : Server Program Number
	//                                         other arguments depend on test case

	//run_mode can switch into stand alone program or program launch by shell script
	//1 : stand alone, debug mode, more screen information
	//0 : launch by shell script as test case, only one printf -> result status
	int run_mode = 0;
	int test_status = 0;	//Default test result set to PASS
	int progNum = atoi(argc[2]);
	int i;
	params paramList[NBCASE];
	CLIENT *clnt = NULL;
	struct sockaddr_in server_addr;
	struct hostent *hp = NULL;
	struct timeval pertry_timeout;
	int sock = RPC_ANYSOCK;

	//Test initialization
	if ((hp = gethostbyname(argc[1])) == NULL) {
		fprintf(stderr, "can't get addr for %s\n", argc[1]);
		exit(-1);
	}

	pertry_timeout.tv_sec = 1;
	pertry_timeout.tv_usec = 0;

	bcopy(hp->h_addr, (caddr_t) & server_addr.sin_addr, hp->h_length);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = 0;

	//Test initialization
	paramList[0].bufsnd = 2147483647;
	paramList[0].bufrec = 0;
	paramList[1].bufsnd = 0;
	paramList[1].bufrec = 2147483647;

	//Call tested function using all tests cases
	for (i = 0; i < NBCASE; i++) {
		//Debug mode prints
		if (run_mode == 1) {
			printf("Test using values : %d ", paramList[i].bufsnd);
			printf("%d", paramList[i].bufrec);
			printf("\n");
		}
		//Call function
		clnt =
		    clntudp_bufcreate(&server_addr, progNum, VERSNUM,
				      pertry_timeout, &sock,
				      paramList[i].bufsnd, paramList[i].bufrec);

		//Check result
		if (clnt == NULL) {
			//test has failed
			test_status = 1;
			break;
		}
	}

	//This last printf gives the result status to the tests suite
	//normally should be 0: test has passed or 1: test has failed
	printf("%d\n", test_status);

	return test_status;
}
