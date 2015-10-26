/*
 * cfgparser.c : GeeXboX uShare config file parser.
 * Originally developped for the GeeXboX project.
 * Copyright (C) 2005-2007 Alexis Saettler <asbin@asbin.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <limits.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <unistd.h>
#include "config.h"
#include "gettext.h"
#include "cfgparser.h"
#include "ushare.h"
#include "trace.h"
#include "osdep.h"

#define USHARE_DIR_DELIM ","

static bool
ignore_line (const char *line)
{
  int i;
  size_t len;

  /* commented line */
  if (line[0] == '#' )
    return true;

  len = strlen (line);

  for (i = 0 ; i < (int) len ; i++ )
    if (line[i] != ' ' && line[i] != '\t' && line[i] != '\n')
      return false;

  return true;
}

static int
ushare_set_dir (struct ushare_t *ut, const char *dirlist)
{
  struct stat buf;
  
  if (!ut)
    return -1;
  
  if (dirlist == NULL)
  	return -1;
  
  if(strcmp(dirlist, DEFAULT_USHARE_STRAGE_NONE) == 0) {
	ut->on = 0;
    return 0;
  }
  else if(strcmp(dirlist, DEFAULT_USHARE_STRAGE_ALL) == 0) {
    strcpy(ut->content_rootdir, "/mnt/shared");
  }
  else {
    sprintf(ut->content_rootdir, "/mnt/shared/%s", dirlist);
  }

  if(stat(ut->content_rootdir, &buf) < 0) {
	return -1;
  }

  if (!ut->contentlist)
  {
    ut->contentlist = (content_list*) malloc (sizeof(content_list));
    ut->contentlist->content = NULL;
    ut->contentlist->count = 0;
  }

  ut->contentlist->count++;
  ut->contentlist->content = (char**) realloc (ut->contentlist->content, ut->contentlist->count * sizeof(char*));
  if (!ut->contentlist->content)
  {
    perror ("error realloc");
    return -1;
  }

  ut->on = 1;
  ut->contentlist->content[ut->contentlist->count-1] = ut->content_rootdir;
  return 0;
}

static int
ushare_set_hostname (struct ushare_t *ut, const char *hostname)
{
  if (!ut || !hostname)
    return -1;
 
  if (ut->name)
    free (ut->name);

  if(!(ut->name = strdup (hostname)))
  	return -1;

  return 0;
}

static int
ushare_set_port (struct ushare_t *ut, const char *port)
{
  if (!ut || !port)
    return -1;

  if(atoi(port) > 0) {
    ut->port = atoi (port);
  }
  else {
    ut->port = DEFAULT_USHARE_PORT;
  }
  
  return 0;
}

static int
ushare_set_mname (struct ushare_t *ut, const char *mname)
{
  if (!ut || !mname)
    return -1;
  
  if (ut->model_name)
    free (ut->model_name);

  if (ut->friendly_name)
    free (ut->friendly_name);

  if(!(ut->model_name = strdup (mname)))
    return -1;

  if(!(ut->friendly_name = strdup (mname)))
    return -1;

  return 0;
}

static int
ushare_set_manufactname (struct ushare_t *ut, const char *mfname)
{
  if (!ut || !mfname)
    return -1;
  
  if (ut->manufact_name)
    free (ut->manufact_name);

  if(!(ut->manufact_name = strdup (mfname)))
    return -1;

  return 0;
}

static int
ushare_set_serialnumber (struct ushare_t *ut, const char *serial)
{
  if (!ut || !serial)
    return -1;
  
  if (ut->serialnum)
    free (ut->serialnum);

  if(!(ut->serialnum = strdup (serial)))
    return -1;
  
  return 0;
}

static int
ushare_set_manufacturl (struct ushare_t *ut, const char *url)
{
  if (!ut || !url)
    return -1;
  
  if (ut->manufact_url)
    free (ut->manufact_url);

  if(!(ut->manufact_url = strdup (url)))
    return -1;
  
  return 0;
}
#if 0
static int
ushare_use_xbox (struct ushare_t *ut, const char *val)
{
  if (!ut || !val)
    return -1;

  ut->xbox360 = (!strcmp (val, "yes")) ? true : false;

  return 0;
}

static int
ushare_use_dlna (struct ushare_t *ut, const char *val)
{
  if (!ut || !val)
    return -1;

#ifdef HAVE_DLNA
  ut->dlna_enabled = (!strcmp (val, "yes")) ? true : false;
#endif /* HAVE_DLNA */
  
  return 0;
}

static int
ushare_set_override_iconv_err (struct ushare_t *ut, const char *arg)
{
  if (!ut)
    return -1;

  ut->override_iconv_err = false;

  if (!strcasecmp (arg, "yes")
      || !strcasecmp (arg, "true")
      || !strcmp (arg, "1"))
    ut->override_iconv_err = true;
    
    return 0;
}
static u_configline_t configline[] = {
  { USHARE_PORT,                 ushare_set_port                },
  { USHARE_DIR,                  ushare_set_dir                 },
  { USHARE_OVERRIDE_ICONV_ERR,   ushare_set_override_iconv_err  },
  { USHARE_ENABLE_XBOX,          ushare_use_xbox                },
  { USHARE_ENABLE_DLNA,          ushare_use_dlna                },
  { USHARE_HOSTNAME,             ushare_set_hostname            },
  { NULL,                        NULL                           },
};
#endif

static u_configline_t configline[] = {
  { "MANUFACTUREURL",                ushare_set_manufacturl     },
/*  { "SERIALNUMBER",                  ushare_set_serialnumber    },*/
  { "MANUFACTURE",                   ushare_set_manufactname    },
  { NULL,                        NULL                           },
};


static void
parse_config_line (struct ushare_t *ut, const char *line,
                   u_configline_t *configline)
{
  char *s = NULL;
  int i = 0;

  s = strchr (line, '=');
  if(s) {
    for (i=0 ; configline[i].name ; i++)
    {
      if (!strncmp (line, configline[i].name, strlen (configline[i].name)))
      {
        configline[i].set_var (ut, s + 1);
        break;
      }
    }
  }
}

#if 1
int
parse_config_file (struct ushare_t *ut)
{
  FILE *conffile;
  char *line = NULL;
  size_t size = 0;
  ssize_t read;
  
  if (!ut)
    return -1;

  conffile = fopen (DEFAULT_CONFIG_FILE, "r");
  if (!conffile)
    return -1;

  while ((read = getline (&line, &size, conffile)) != -1)
  {
    if (ignore_line (line))
      continue;

    if (line[read-1] == '\n')
      line[read-1] = '\0';

    while (line[0] == ' ' || line[0] == '\t')
      line++;
    
    parse_config_line (ut, line, configline);
  }

  fclose (conffile);

  if (line)
    free (line);

  return 0;
}
#endif

inline static void
display_usage (void)
{
  display_headers ();
  printf ("\n");
  printf (_("Usage: ushare [-n name] [-i interface] [-p port] [-c directory] [[-c directory]...]\n"));
  printf (_("Options:\n"));
  printf (_(" -o, --override-iconv-err(no use)\t"
  "If iconv fails parsing name, still add to media contents (hoping the renderer can handle it)\n"));
  printf (_(" -v, --verbose\t\tSet verbose display(use)\n"));
  printf (_(" -x, --xbox\t\tUse XboX 360 compliant profile(no use)\n"));
  printf (_(" -m, --model\t\tDefault \"Media Server\"\n"));
  printf (_(" -n, --name\t\tDefault [hostname]\n"));
  printf (_(" -p, --port\t\tDLNA use port\n"));
  printf (_(" -c, --directory\t\tDLNA use directory\n"));
  printf (_(" -s, --sirial number\t\tUPnP use sirial number\n"));
#ifdef HAVE_DLNA
  printf (_(" -d, --dlna\t\tUse DLNA compliant profile(use)\n"));
#endif /* HAVE_DLNA */
  printf (_(" -D, --daemon\t\tRun as a daemon(use)\n"));
  printf (_(" -V, --version\t\tDisplay the version of uShare and exit\n"));
  printf (_(" -h, --help\t\tDisplay this help\n"));
}

int
parse_command_line (struct ushare_t *ut, int argc, char **argv)
{
  int c, index;
  int retval = 0;
  char short_options[] = "VhvDoxdp:c:m:n:s:";
  struct option long_options [] = {
    {"override-iconv-err", no_argument, 0, 'o' },
    {"verbose", no_argument, 0, 'v' },
    {"xbox", no_argument, 0, 'x' },
    {"model", no_argument, 0, 'm' },
    {"name", no_argument, 0, 'n' },
    {"port", no_argument, 0, 'p' },
    {"sirial", no_argument, 0, 's' },
    {"direcotry", no_argument, 0, 'c' },
#ifdef HAVE_DLNA
    {"dlna", no_argument, 0, 'd' },
#endif /* HAVE_DLNA */
    {"daemon", no_argument, 0, 'D' },
    {"version", no_argument, 0, 'V' },
    {"help", no_argument, 0, 'h' },
    {0, 0, 0, 0 }
  };

  /* command line argument processing */
  while (true)
  {
    c = getopt_long (argc, argv, short_options, long_options, &index);

    if (c == EOF)
      break;

    switch (c)
    {
    case 0:
      /* opt = long_options[index].name; */
      break;

    case 'o':
      ut->override_iconv_err = false;
      break;

    case 'v':
      ut->verbose = true;
      break;

    case 'x':
      ut->xbox360 = false;
      break;

    case 'm':
      if(ushare_set_mname(ut, optarg) < 0)
        goto ERR_POINT;
      break;

    case 'n':
      if(ushare_set_hostname(ut, optarg) < 0)
        goto ERR_POINT;
      break;

    case 'p':
      if(ushare_set_port (ut, optarg) < 0)
        goto ERR_POINT;
      break;

    case 's':
      if(ushare_set_serialnumber (ut, optarg) < 0)
        goto ERR_POINT;
      break;

    case 'c':
      if(ushare_set_dir (ut, optarg) < 0)
      	goto ERR_POINT;
      break;

#ifdef HAVE_DLNA
    case 'd':
      ut->dlna_enabled = true;
      break;
#endif /* HAVE_DLNA */

    case 'D':
      ut->daemon = true;
      break;

    case 'V':
      display_headers ();
      return -1;

    case '?':
    case 'h':
      display_usage ();
      return -1;

    default:
      break;
    }
  }

END_POINT:
  return retval;
  
ERR_POINT:
  retval = -1;
  goto END_POINT;
}
