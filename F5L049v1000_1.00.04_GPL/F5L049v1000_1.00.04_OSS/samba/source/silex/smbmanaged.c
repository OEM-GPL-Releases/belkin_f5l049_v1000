/**
 * @file
 *
 * samba manager.
 *
 *
 * Copyright (C) 2008 by silex technology, Inc.
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License 
 * any later version.
**/
#include "includes.h"

#define PROGNAME(name) progname(*argv,name)

/**
 * check program name.
 *
 * @param[in] arg  command line arguments.
 * @return  1 is same
 * @return  0 is different.
**/
int progname(const char *arg,const char *progname)
{
	char *p;
	if((p=strstr(arg,progname))){
		int len = strlen(progname);
		if(((p>arg&&*(p-1)=='/')||p==arg)&&p[len]=='\0')
			return 1;
	}
	return 0;
}

/**
 * program entory point.
 *
 * @param[in] argc  number of argument.
 * @param[in] argv  command line arguments.
 * @return  error code.
**/
int main(int argc,char **argv)
{
	if(PROGNAME("nmbd"))      return nmbd_main(argc,argv);
	if(PROGNAME("smbd"))      return smbd_main(argc,argv);
	if(PROGNAME("smbpasswd")) return smbpasswd_main(argc,argv);

	printf("Error : no set program name.\n");
	return -1;
}

