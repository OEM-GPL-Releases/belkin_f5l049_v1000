/*
 * metadata.c : GeeXboX uShare CDS Metadata DB.
 * Originally developped for the GeeXboX project.
 * Copyright (C) 2005-2007 Benjamin Zores <ben@geexbox.org>
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <unistd.h>
#include <upnp/upnp.h>
#include <upnp/upnptools.h>
#include <sys/time.h>

#include "mime.h"
#include "metadata.h"
#include "util_iconv.h"
#include "content.h"
#include "gettext.h"
#include "trace.h"

#define TITLE_UNKNOWN "unknown"

#define MAX_URL_SIZE 32
#define DEBUG_TIME 0

struct upnp_entry_lookup_t {
  int id;
  struct upnp_entry_t *entry_ptr;
};

static char *
getExtension (const char *filename)
{
  char *str = NULL;

  str = strrchr (filename, '.');
  if (str)
    str++;

  return str;
}

static struct mime_type_t *
getMimeType (const char *extension)
{
  extern struct mime_type_t MIME_Type_List[];
  struct mime_type_t *list;

  if (!extension)
    return NULL;

  list = MIME_Type_List;
  while (list->extension)
  {
    if (!strcasecmp (list->extension, extension))
      return list;
    list++;
  }

  return NULL;
}

int
get_fullpath (struct upnp_entry_t *entry, char **fullpathp)
{
  struct upnp_entry_t *en = NULL;
  char *path = NULL, *buf = NULL, *fullpath = NULL;
  int retval = 0;
  int extensionlen = 0;

  if(entry->extension != NULL) {
	extensionlen = 1;
  }
  else {
	extensionlen = (strlen(entry->extension) + strlen(entry->mime_type->extension));
  }
  
  en = entry;
  while(en->parent) {
    int len = 0;
    struct upnp_entry_t *tmp = NULL;
    char *strp = NULL;

    if(en->filename != NULL)
      strp = en->filename;
    else
      strp = en->title;

    if(path) {
      buf = malloc(sizeof(char) * strlen(path)+2);
      if(!buf)
		goto ERR_POINT;
      
      memset(buf, 0x00, strlen(path)+2);
      strcpy(buf, path);
      free(path);
      len = strlen(buf);
    }

	path = malloc(sizeof(char) * (strlen(strp) + len + 2));
	if(!path)
		goto ERR_POINT;

    memset(path, 0x00, (strlen(strp) + len + 2));
	if(buf)
		sprintf(path, "%s/%s", strp, buf);
	else
		sprintf(path, "%s", strp);
		
	free(buf);
	tmp = en;
	en = en->parent;
	if(!en->parent) {

		buf = malloc(sizeof(char) * (strlen(tmp->fullpath) + 1));
		if(!buf)
			goto ERR_POINT;

        memset(buf, 0x00, (strlen(tmp->fullpath) + 1));
		strcpy(buf, tmp->fullpath);
		len = strlen(buf);
		fullpath = malloc(sizeof(char) * (strlen(path) + len + extensionlen + 3));

		if(!fullpath)
			goto ERR_POINT;

        memset(fullpath, 0x00, (strlen(path) + len + extensionlen + 3));

        if(entry->extension != NULL) {
				sprintf(fullpath, "%s/%s.%s", buf, &path[strlen(tmp->title)+1], entry->extension);
				/* &path[strlen(tmp->title)+1] <- Music or Video or Pictures */
		}
		else {
			sprintf(fullpath, "%s/%s", buf, &path[strlen(tmp->title)+1]);
		}
		*fullpathp = strdup (fullpath);
	}
  }

END_POINT:
	if(path)
		free(path);
	if(buf)
		free(buf);
	if(fullpath)
		free(fullpath);
	
	return retval;

ERR_POINT:
	retval = -1;
	goto END_POINT;
}

static bool
is_valid_extension (const char *extension)
{
  if (!extension)
    return false;

  if (getMimeType (extension))
    return true;

  return false;
}

static int
get_list_length (void *list)
{
  void **l = list;
  int n = 0;

  while (*(l++))
    n++;

  return n;
}

static xml_convert_t xml_convert[] = {
  {'"' , "&quot;"},
  {'&' , "&amp;"},
  {'\'', "&apos;"},
  {'<' , "&lt;"},
  {'>' , "&gt;"},
  {'\n', "&#xA;"},
  {'\r', "&#xD;"},
  {'\t', "&#x9;"},
  {0, NULL},
};

static char *
get_xmlconvert (int c)
{
  int j;
  for (j = 0; xml_convert[j].xml; j++)
  {
    if (c == xml_convert[j].charac)
      return xml_convert[j].xml;
  }
  return NULL;
}

static char *
convert_xml (const char *title)
{
  char *newtitle, *s, *t, *xml;
  int nbconvert = 0;

  /* calculate extra size needed */
  for (t = (char*) title; *t; t++)
  {
    xml = get_xmlconvert (*t);
    if (xml)
      nbconvert += strlen (xml) - 1;
  }
  if (!nbconvert)
    return NULL;

  newtitle = s = (char*) malloc (strlen (title) + nbconvert + 1);

  for (t = (char*) title; *t; t++)
  {
    xml = get_xmlconvert (*t);
    if (xml)
    {
      strcpy (s, xml);
      s += strlen (xml);
    }
    else
      *s++ = *t;
  }
  *s = '\0';

  return newtitle;
}

static struct mime_type_t Container_MIME_Type =
  { NULL, "object.container.storageFolder", NULL};

static struct upnp_entry_t *
upnp_entry_new (struct ushare_t *ut, const char *name, const char *fullpath,
                struct upnp_entry_t *parent, off_t size, int dir)
{
  struct upnp_entry_t *entry = NULL, *en = NULL;
  char *title = NULL, *x = NULL;
  char url_tmp[MAX_URL_SIZE] = { '\0' };
  char *title_or_name = NULL;
  char *cp;

  if (!name)
    return NULL;

  entry = (struct upnp_entry_t *) malloc (sizeof (struct upnp_entry_t));
  /* fullpath -> directory & contents */

#ifdef HAVE_DLNA
  entry->dlna_profile = NULL;
  entry->url = NULL;
  if (ut->dlna_enabled && fullpath && !dir)
  {
    dlna_profile_t *p = dlna_guess_media_profile (ut->dlna, fullpath);
    if (!p)
    {
      free (entry);
      return NULL;
    }
    entry->dlna_profile = p;
  }
#endif /* HAVE_DLNA */
 
  if (ut->xbox360)
  {
    if (ut->root_entry)
      entry->id = ut->starting_id + ut->nr_entries++;
    else
      entry->id = 0; /* Creating the root node so don't use the usual IDs */
  }
  else
    entry->id = ut->starting_id + ut->nr_entries++;
  
  if(fullpath) {
	  en = parent;
	  if(!en->parent) {
		entry->fullpath = fullpath ? strdup (fullpath) : NULL;
	  }
	  else
	  	entry->fullpath = NULL;
  }
  entry->parent = parent;
  entry->child_count =  dir ? 0 : -1;
  entry->title = NULL;
  entry->extension = NULL;
  entry->filename = NULL;

  entry->childs = (struct upnp_entry_t **)
    malloc (sizeof (struct upnp_entry_t *));
  *(entry->childs) = NULL;

  if (!dir) /* item */
    {
#ifdef HAVE_DLNA
      if (ut->dlna_enabled)
        entry->mime_type = NULL;
      else
      {
#endif /* HAVE_DLNA */
      struct mime_type_t *mime = getMimeType (getExtension (name));

      /* save file extension */
      if ((cp = getExtension(name)) != NULL) {
        entry->extension = (char *)malloc(strlen(cp) + 1);
        strcpy(entry->extension, cp);
      }

      if (!mime)
      {
        --ut->nr_entries; 
        upnp_entry_free (ut, entry);
        log_error ("Invalid Mime type for %s, entry ignored", name);
        return NULL;
      }
      entry->mime_type = mime;
#ifdef HAVE_DLNA
      }
#endif /* HAVE_DLNA */
      
      if (snprintf (url_tmp, MAX_URL_SIZE, "%d.%s",
                    entry->id, getExtension (name)) >= MAX_URL_SIZE)
        log_error ("URL string too long for id %d, truncated!!", entry->id);

        for (cp = url_tmp; *cp != '\0'; cp++) *cp = tolower(*cp);	/* tolower */

      /* Only malloc() what we really need */
      entry->url = strdup (url_tmp);
    }
  else /* container */
    {
      entry->mime_type = &Container_MIME_Type;
      entry->url = NULL;
    }

  /* Try Iconv'ing the name but if it fails the end device
     may still be able to handle it */
  title = iconv_convert (name);
  if (title)
    title_or_name = title;
  else
  {
    if (ut->override_iconv_err)
    {
      title_or_name = strdup (name);
    }
    else
    {
      upnp_entry_free (ut, entry);
      log_error ("Freeing entry invalid name id=%d [%s]\n", entry->id, name);
      return NULL;
    }
  }

  if (!dir)
  {
    x = strrchr (title_or_name, '.');
    if (x)  /* avoid displaying file extension */
      *x = '\0';
  }
  x = convert_xml (title_or_name);
  if (x)
  {
    entry->filename = title_or_name;
    title_or_name = x;
  }
  entry->title = title_or_name;

  if (!strcmp (title_or_name, "")) /* DIDL dc:title can't be empty */
  {
    free (title_or_name);
    entry->title = strdup (TITLE_UNKNOWN);
  }

  entry->size = size;
  entry->fd = -1;

  if (entry->id && entry->url) {
    log_verbose ("Entry->URL (%d): %s\n", entry->id, entry->url);
  }

  return entry;
}

/* Seperate recursive free() function in order to avoid freeing off
 * the parents child list within the freeing of the first child, as
 * the only entry which is not part of a childs list is the root entry
 */
static void
_upnp_entry_free (struct upnp_entry_t *entry)
{
  struct upnp_entry_t **childs;

  if (!entry)
    return;

  if (entry->fullpath)
    free (entry->fullpath);
  if (entry->title)
    free (entry->title);
  if (entry->filename)
    free (entry->filename);
  if (entry->url)
    free (entry->url);
#ifdef HAVE_DLNA
  if (entry->dlna_profile)
    entry->dlna_profile = NULL;
#endif /* HAVE_DLNA */
  if (entry->extension) free(entry->extension);

  for (childs = entry->childs; *childs; childs++)
    _upnp_entry_free (*childs);
  free (entry->childs);
}

void
upnp_entry_free (struct ushare_t *ut, struct upnp_entry_t *entry)
{
  if (!ut || !entry)
    return;

  /* Free all entries (i.e. children) */
  if (entry == ut->root_entry)
  {
    struct upnp_entry_t *entry_found = NULL;
    struct upnp_entry_lookup_t *lk = NULL;
    RBLIST *rblist;
    int i = 0;

    rblist = rbopenlist (ut->rb);
    lk = (struct upnp_entry_lookup_t *) rbreadlist (rblist);

    while (lk)
    {
      entry_found = lk->entry_ptr;
      if (entry_found)
      {
 	if (entry_found->fullpath)
 	  free (entry_found->fullpath);
 	if (entry_found->title)
 	  free (entry_found->title);
 	if (entry_found->url)
 	  free (entry_found->url);

	free (entry_found);
 	i++;
      }

      free (lk); /* delete the lookup */
      lk = (struct upnp_entry_lookup_t *) rbreadlist (rblist);
    }

    rbcloselist (rblist);
    rbdestroy (ut->rb);
    ut->rb = NULL;

    log_verbose ("Freed [%d] entries\n", i);
  }
  else
    _upnp_entry_free (entry);

  free (entry);
}


static void
upnp_entry_add_child (struct ushare_t *ut,
                      struct upnp_entry_t *entry, struct upnp_entry_t *child)
{
  struct upnp_entry_lookup_t *entry_lookup_ptr = NULL;
  struct upnp_entry_t **childs;
  int n;

  if (!entry || !child)
    return;

  for (childs = entry->childs; *childs; childs++)
    if (*childs == child)
      return;

  n = get_list_length ((void *) entry->childs) + 1;
  entry->childs = (struct upnp_entry_t **)
    realloc (entry->childs, (n + 1) * sizeof (*(entry->childs)));
  entry->childs[n] = NULL;
  entry->childs[n - 1] = child;
  entry->child_count++;

  entry_lookup_ptr = (struct upnp_entry_lookup_t *)
    malloc (sizeof (struct upnp_entry_lookup_t));
  entry_lookup_ptr->id = child->id;
  entry_lookup_ptr->entry_ptr = child;

  if (rbsearch ((void *) entry_lookup_ptr, ut->rb) == NULL)
    log_info (_("Failed to add the RB lookup tree\n"));
}

struct upnp_entry_t *
upnp_get_entry (struct ushare_t *ut, int id)
{
  struct upnp_entry_lookup_t *res, entry_lookup;

  log_verbose ("Looking for entry id %d\n", id);
  if (id == 0) /* We do not store the root (id 0) as it is not a child */
    return ut->root_entry;

  entry_lookup.id = id;
  res = (struct upnp_entry_lookup_t *)
    rbfind ((void *) &entry_lookup, ut->rb);

  if (res)
  {
    log_verbose ("Found at %p\n",
                 ((struct upnp_entry_lookup_t *) res)->entry_ptr);
    return ((struct upnp_entry_lookup_t *) res)->entry_ptr;
  }

  log_verbose ("Not Found\n");

  return NULL;
}

/* Contents file add. */
static int
metadata_add_file (struct ushare_t *ut, struct upnp_entry_t *entry,
                   const char *file, const char *name, struct stat *st_ptr, int *num)
{
	if (!entry || !file || !name)
		return -1;

#ifdef HAVE_DLNA
	if (ut->dlna_enabled || is_valid_extension (getExtension (file))) {
#else
	if (is_valid_extension (getExtension (file))) {
#endif
		struct upnp_entry_t *child = NULL;

		if((*num) == 0)
			return 1;
  
		child = upnp_entry_new (ut, name, file, entry, st_ptr->st_size, false);
		--(*num);

		if (child)
			upnp_entry_add_child (ut, entry, child);
	}

	return 0;
}

/* Contents file add. */
#if 0
static int
metadata_add_container (struct ushare_t *ut,
                        struct upnp_entry_t *entry, const char *container)
#else
static int
metadata_add_container (struct ushare_t *ut,
                        struct upnp_entry_t *entry, const char *container,
                        int media_type, int *num)
#endif
{
  DIR *dir;
  struct stat st;
  struct dirent d;
  struct dirent *result;
  int retval = 0;

  if (!entry || !container)
    return -1;

  if((dir=opendir(container))==NULL) {
    perror("opendir: ");
    log_error (_("No directory open. (%s)\n"), container);
    return -1;
  }

	for(readdir_r(dir, &d, &result);result!=NULL;readdir_r(dir, &d, &result)) {
		char *fullpath;

		if (d.d_name[0] == '.') {
			continue;
		}

		fullpath = (char *)malloc(strlen(container) + 2 + strlen(d.d_name));
		if(!fullpath) {
			perror("malloc error: ");
			retval = -1;
			goto END_POINT;
		}

		sprintf(fullpath, "%s/%s", container, d.d_name);
		log_verbose ("%s\n", fullpath);

		if (stat (fullpath, &st) < 0) {
			perror("No file: ");
			free(fullpath);
			continue;
		}

	   /* Add directory. */
		if (S_ISDIR (st.st_mode)) {
			struct upnp_entry_t *child = NULL;

				child = upnp_entry_new (ut, d.d_name, fullpath, entry, 0, true);
				--(*num);
				if((*num) == 0) {
				 	if(child)
					 	upnp_entry_add_child (ut, entry, child);
					retval = 1;
					free(fullpath);
					goto END_POINT;
				}

				if (child) {
					if((retval = metadata_add_container (ut, child, fullpath,
					 media_type, num)) != 0) {
					
						/* Malloc error.(-1), contents max(1) */
						log_error (_("retval(%d) \n"), retval);

						if(retval > 0) /* Maxdata */
						 	upnp_entry_add_child (ut, entry, child);

						free(fullpath);
						goto END_POINT;
					}
					upnp_entry_add_child (ut, entry, child);
				}
		/*	} */
			free(fullpath);
			continue;
		}
	   else {
#if 1
			struct mime_type_t *mime;
			int r;

			if ((mime = getMimeType(getExtension(d.d_name))) != NULL) {
				r = 0;
				switch (media_type) {
				case -1: /* All */
					r = 1;
					break;

				case 0: /* Music */
					if (strstr(mime->mime_protocol, "audio") != NULL) r = 1;
					break;

				case 1: /* Videos */
					if (strstr(mime->mime_protocol, "video") != NULL) r = 1;
					break;

				case 2: /* Pictures */
					if (strstr(mime->mime_protocol, "image") != NULL) r = 1;
					break;
				}
				/* if (strstr(mime->mime_protocol, "text") != NULL) r = 1	*/

				if(r) {
					if((retval = metadata_add_file (ut, entry, fullpath, d.d_name, &st, num)) != 0) {
						log_error (_("retval(%d) \n"), retval);
						retval = 1;
						free(fullpath);
						goto END_POINT;
					}
				}
			}
#else
     if((retval = metadata_add_file (ut, entry, fullpath, d.d_name, &st)) != 0) {
       log_error (_("retval(%d) \n"), retval);
       retval = 1;
       free(fullpath);
       goto END_POINT;
     }
#endif
		}
		
		free(fullpath);
	}

END_POINT:
	closedir(dir);
	return retval;
}

void
free_metadata_list (struct ushare_t *ut)
{
  ut->init = 0;
  if (ut->root_entry)
    upnp_entry_free (ut, ut->root_entry);
  ut->root_entry = NULL;
  ut->nr_entries = 0;

  if (ut->rb)
  {
    rbdestroy (ut->rb);
    ut->rb = NULL;
  }

  ut->rb = rbinit (rb_compare, NULL);
  if (!ut->rb)
    log_error (_("Cannot create RB tree for lookups\n"));
}

int
build_metadata_list (struct ushare_t *ut)
{
  int i;
  int retval = 0;
#if DEBUG_TIME
  clock_t start_time, end_time;
#endif

  /* build root entry */
  if (!ut->root_entry) {
    ut->root_entry = upnp_entry_new (ut, "root", NULL, NULL, -1, true);
    }

  /* add files from content directory */
  for (i=0 ; i < ut->contentlist->count ; i++) /* count -> root_directory */
  {
    struct upnp_entry_t *entry = NULL;
    int size = 0;
#if 0
    char *title = NULL;
#endif

    log_info (_("Looking for files in content directory : %s\n"),
              ut->contentlist->content[i]);

    size = strlen (ut->contentlist->content[i]);
    if (ut->contentlist->content[i][size - 1] == '/')
      ut->contentlist->content[i][size - 1] = '\0';
#if 1
	/* for Xbox360 */
	/* virtual media library folders */
	{
		static const char *top_titles[] = { "Music", "Videos", "Pictures" };
		static int top_ids[] = { 4, 15, 16 };	/* fixed ids */
		int n;

		for (n = 0; n < 3; n++) {
			if ((entry = upnp_entry_new(ut, top_titles[n], ut->contentlist->content[i], ut->root_entry, -1, true)) != NULL) {
				int num = (1500+1);
/*				int num = (5+1);*/

				entry->id = top_ids[n];
				upnp_entry_add_child(ut, ut->root_entry, entry);

				if((retval = metadata_add_container(ut, entry, ut->contentlist->content[i], n, &num)) < 0) {
					/* Malloc error.(-1), contents max(1) */
					log_error (_("retval(%d)\n"), retval);
					goto END_POINT;
				}
			/*	if (retval > 0) break; */
			}
		}
	}
#else
    title = strrchr (ut->contentlist->content[i], '/');
    if (title)
      title++;
    else
    {
      /* directly use content directory name if no '/' before basename */
      title = ut->contentlist->content[i];
    }

	/* title , ut->contentlist->content,  ->root directory. */
    entry = upnp_entry_new (ut, title, ut->contentlist->content[i],
                            ut->root_entry, -1, true);

    if (!entry)
      continue;

    upnp_entry_add_child (ut, ut->root_entry, entry);
#if DEBUG_TIME
	/* time set */
	start_time = clock();
#endif
    if((retval = metadata_add_container (ut, entry, ut->contentlist->content[i], -1)) < 0) {
		/* Malloc error.(-1), contents max(1) */
        log_error (_("retval(%d)\n"), retval);
		goto END_POINT;
	}
	if(retval > 0)
		break;
#endif
  }

  log_info (_("Found %d files and subdirectories.\n"), ut->nr_entries);
  ut->init = 1;

END_POINT:
#if DEBUG_TIME
	end_time = clock();
	printf("\n %10.30f sec\n",(double)(end_time - start_time)/CLOCKS_PER_SEC);
#endif
  return retval;
}

int
rb_compare (const void *pa, const void *pb,
            const void *config __attribute__ ((unused)))
{
  struct upnp_entry_lookup_t *a, *b;

  a = (struct upnp_entry_lookup_t *) pa;
  b = (struct upnp_entry_lookup_t *) pb;

  if (a->id < b->id)
    return -1;

  if (a->id > b->id)
    return 1;

  return 0;
}

