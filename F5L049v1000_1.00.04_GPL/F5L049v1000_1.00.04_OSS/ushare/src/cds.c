/*
 * cds.c : GeeXboX uShare Content Directory Service
 * Originally developped for the GeeXboX project.
 * Parts of the code are originated from GMediaServer from Oskar Liljeblad.
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
#include <sys/stat.h>
#include <time.h>
#include <upnp/upnp.h>
#include <upnp/upnptools.h>

#include "ushare.h"
#include "services.h"
#include "ushare.h"
#include "services.h"
#include "metadata.h"
#include "mime.h"
#include "buffer.h"
#include "minmax.h"

/* Represent the CDS GetSearchCapabilities action. */
#define SERVICE_CDS_ACTION_SEARCH_CAPS "GetSearchCapabilities"

/* Represent the CDS GetSortCapabilities action. */
#define SERVICE_CDS_ACTION_SORT_CAPS "GetSortCapabilities"

/* Represent the CDS GetSystemUpdateID action. */
#define SERVICE_CDS_ACTION_UPDATE_ID "GetSystemUpdateID"

/* Represent the CDS Browse action. */
#define SERVICE_CDS_ACTION_BROWSE "Browse"

/* Represent the CDS Search action. */
#define SERVICE_CDS_ACTION_SEARCH "Search"

/* Represent the CDS SearchCaps argument. */
#define SERVICE_CDS_ARG_SEARCH_CAPS "SearchCaps"

/* Represent the CDS SortCaps argument. */
#define SERVICE_CDS_ARG_SORT_CAPS "SortCaps"

/* Represent the CDS UpdateId argument. */
#define SERVICE_CDS_ARG_UPDATE_ID "Id"

/* Represent the CDS StartingIndex argument. */
#define SERVICE_CDS_ARG_START_INDEX "StartingIndex"

/* Represent the CDS RequestedCount argument. */
#define SERVICE_CDS_ARG_REQUEST_COUNT "RequestedCount"

/* Represent the CDS ObjectID argument. */
#define SERVICE_CDS_ARG_OBJECT_ID "ObjectID"

/* Represent the CDS Filter argument. */
#define SERVICE_CDS_ARG_FILTER "Filter"

/* Represent the CDS BrowseFlag argument. */
#define SERVICE_CDS_ARG_BROWSE_FLAG "BrowseFlag"

/* Represent the CDS SortCriteria argument. */
#define SERVICE_CDS_ARG_SORT_CRIT "SortCriteria"

/* Represent the CDS SearchCriteria argument. */
#define SERVICE_CDS_ARG_SEARCH_CRIT "SearchCriteria"

/* Represent the CDS Root Object ID argument. */
#define SERVICE_CDS_ROOT_OBJECT_ID "0"

/* Represent the CDS DIDL Message Metadata Browse flag argument. */
#define SERVICE_CDS_BROWSE_METADATA "BrowseMetadata"

/* Represent the CDS DIDL Message DirectChildren Browse flag argument. */
#define SERVICE_CDS_BROWSE_CHILDREN "BrowseDirectChildren"

/* Represent the CDS DIDL Message Result argument. */
#define SERVICE_CDS_DIDL_RESULT "Result"

/* Represent the CDS DIDL Message NumberReturned argument. */
#define SERVICE_CDS_DIDL_NUM_RETURNED "NumberReturned"

/* Represent the CDS DIDL Message TotalMatches argument. */
#define SERVICE_CDS_DIDL_TOTAL_MATCH "TotalMatches"

/* Represent the CDS DIDL Message UpdateID argument. */
#define SERVICE_CDS_DIDL_UPDATE_ID "UpdateID"

/* DIDL parameters */
/* Represent the CDS DIDL Message Header Namespace. */
#define DIDL_NAMESPACE \
    "xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\" " \
    "xmlns:dc=\"http://purl.org/dc/elements/1.1/\" " \
    "xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\""

/* Represent the CDS DIDL Message Header Tag. */
#define DIDL_LITE "DIDL-Lite"

/* Represent the CDS DIDL Message Item value. */
#define DIDL_ITEM "item"

/* Represent the CDS DIDL Message Item ID value. */
#define DIDL_ITEM_ID "id"

/* Represent the CDS DIDL Message Item Parent ID value. */
#define DIDL_ITEM_PARENT_ID "parentID"

/* Represent the CDS DIDL Message Item Restricted value. */
#define DIDL_ITEM_RESTRICTED "restricted"

/* Represent the CDS DIDL Message Item UPnP Class value. */
#define DIDL_ITEM_CLASS "upnp:class"

/* Represent the CDS DIDL Message Item Title value. */
#define DIDL_ITEM_TITLE "dc:title"

/* Represent the CDS DIDL Message Item Resource value. */
#define DIDL_RES "res"

/* Represent the CDS DIDL Message Item Protocol Info value. */
#define DIDL_RES_INFO "protocolInfo"

/* Represent the CDS DIDL Message Item Resource Size value. */
#define DIDL_RES_SIZE "size"

/* Represent the CDS DIDL Message Container value. */
#define DIDL_CONTAINER "container"

/* Represent the CDS DIDL Message Container ID value. */
#define DIDL_CONTAINER_ID "id"

/* Represent the CDS DIDL Message Container Parent ID value. */
#define DIDL_CONTAINER_PARENT_ID "parentID"

/* Represent the CDS DIDL Message Container number of children value. */
#define DIDL_CONTAINER_CHILDS "childCount"

/* Represent the CDS DIDL Message Container Restricted value. */
#define DIDL_CONTAINER_RESTRICTED "restricted"

/* Represent the CDS DIDL Message Container Searchable value. */
#define DIDL_CONTAINER_SEARCH "searchable"

/* Represent the CDS DIDL Message Container UPnP Class value. */
#define DIDL_CONTAINER_CLASS "upnp:class"

/* Represent the CDS DIDL Message Container Title value. */
#define DIDL_CONTAINER_TITLE "dc:title"

/* Represent the CDS DIDL Message Container Date value. */
#define DIDL_CONTAINER_DATE "dc:date"

/* Represent the "upnp:class" reserved keyword for Search action */
#define SEARCH_CLASS_MATCH_KEYWORD "(upnp:class = \""

/* Represent the "upnp:class derived from" reserved keyword */
#define SEARCH_CLASS_DERIVED_KEYWORD "(upnp:class derivedfrom \""

/* Represent the "res@protocolInfo contains" reserved keyword */
#define SEARCH_PROTOCOL_CONTAINS_KEYWORD "(res@protocolInfo contains \""

/* Represent the Search default keyword */
#define SEARCH_OBJECT_KEYWORD "object"

/* Represent the Search 'AND' connector keyword */
#define SEARCH_AND ") and ("

static bool
filter_has_val (const char *filter, const char *val)
{
  char *x = NULL, *token = NULL;
  char *m_buffer = NULL, *buffer;
  int len = strlen (val);
  bool ret = false;

  if (!strcmp (filter, "*"))
    return true;

  x = strdup (filter);
  if (x)
  {
    m_buffer = (char*) malloc (strlen (x));
    if (m_buffer)
    {
      buffer = m_buffer;
      token = strtok_r (x, ",", &buffer);
      while (token)
      {
        if (*val == '@')
          token = strchr (token, '@');
        if (token && !strncmp (token, val, len))
        {
          ret = true;
          break;
        }
        token = strtok_r (NULL, ",", &buffer);
      }
      free (m_buffer);
    }
    free (x);
  }
  return ret;
}

/* UPnP ContentDirectory Service actions */
static bool
cds_get_search_capabilities (struct action_event_t *event)
{
/* upnp_add_response (event, SERVICE_CDS_ARG_SEARCH_CAPS, ""); */
  upnp_add_response (event, SERVICE_CDS_ARG_SEARCH_CAPS, DIDL_CONTAINER_CLASS "," DIDL_CONTAINER_TITLE);

  return event->status;
}

static bool
cds_get_sort_capabilities (struct action_event_t *event)
{
/* upnp_add_response (event, SERVICE_CDS_ARG_SORT_CAPS, ""); */
  upnp_add_response (event, SERVICE_CDS_ARG_SORT_CAPS, DIDL_CONTAINER_CLASS "," DIDL_CONTAINER_TITLE);

  return event->status;
}

static bool
cds_get_system_update_id (struct action_event_t *event)
{
  upnp_add_response (event, SERVICE_CDS_ARG_UPDATE_ID,
                     SERVICE_CDS_ROOT_OBJECT_ID);

  return event->status;
}

static void
didl_add_header (struct buffer_t *out)
{
  buffer_appendf (out, "<%s %s>", DIDL_LITE, DIDL_NAMESPACE);
}

static void
didl_add_footer (struct buffer_t *out)
{
  buffer_appendf (out, "</%s>", DIDL_LITE);
}

static void
didl_add_tag (struct buffer_t *out, char *tag, char *value)
{
  if (value)
    buffer_appendf (out, "<%s>%s</%s>", tag, value, tag);
}

static void
didl_add_param (struct buffer_t *out, char *param, char *value)
{
  if (value)
    buffer_appendf (out, " %s=\"%s\"", param, value);
}

static void
didl_add_value (struct buffer_t *out, char *param, off_t value)
{
  buffer_appendf (out, " %s=\"%lld\"", param, value);
}

static void
didl_add_item (struct buffer_t *out, int item_id,
               int parent_id, char *restricted, char *class, char *title,
               char *protocol_info, off_t size, char *url, char *filter, char *fullpath)
{
  struct stat st;
  struct tm tv;
  char str[256];

  buffer_appendf (out, "<%s", DIDL_ITEM);
  didl_add_value (out, DIDL_ITEM_ID, item_id);
  didl_add_value (out, DIDL_ITEM_PARENT_ID, parent_id);
  didl_add_param (out, DIDL_ITEM_RESTRICTED, restricted);
  buffer_append (out, ">");

  didl_add_tag (out, DIDL_ITEM_CLASS, class);
  didl_add_tag (out, DIDL_ITEM_TITLE, title);

  /* dc:date */
  if(fullpath == NULL) {
    didl_add_tag (out, DIDL_CONTAINER_DATE, "0000-00-00");
  }
  else {
    if(stat(fullpath, &st) != 0) {
      didl_add_tag (out, DIDL_CONTAINER_DATE, "0000-00-00");
    }
    else {
      if(localtime_r (&(st.st_mtime), &tv) == NULL) {
        didl_add_tag (out, DIDL_CONTAINER_DATE, "0000-00-00");
	  }
	  else {
        strftime(str, 255, "%Y-%m-%d", &tv);
        didl_add_tag (out, DIDL_CONTAINER_DATE, str);
      }
    }
  }

  if (filter_has_val (filter, DIDL_RES))
  {
    buffer_appendf (out, "<%s", DIDL_RES);
    // protocolInfo is required :
    didl_add_param (out, DIDL_RES_INFO, protocol_info);
    if (filter_has_val (filter, "@"DIDL_RES_SIZE))
      didl_add_value (out, DIDL_RES_SIZE, size);
    buffer_append (out, ">");
    if (url)
    {
      extern struct ushare_t *ut;
      buffer_appendf (out, "http://%s:%d%s/%s",
                      UpnpGetServerIpAddress (), ut->port, VIRTUAL_DIR, url);
    }
    buffer_appendf (out, "</%s>", DIDL_RES);
  }
  buffer_appendf (out, "</%s>", DIDL_ITEM);
}

static void
didl_add_container (struct buffer_t *out, int id, int parent_id,
                    int child_count, char *restricted, char *searchable,
                    char *title, char *class)
{
  buffer_appendf (out, "<%s", DIDL_CONTAINER);

  didl_add_value (out, DIDL_CONTAINER_ID, id);
  didl_add_value (out, DIDL_CONTAINER_PARENT_ID, parent_id);
  if (child_count >= 0)
    didl_add_value (out, DIDL_CONTAINER_CHILDS, child_count);
  didl_add_param (out, DIDL_CONTAINER_RESTRICTED, restricted);
  didl_add_param (out, DIDL_CONTAINER_SEARCH, searchable);
  buffer_append (out, ">");

  didl_add_tag (out, DIDL_CONTAINER_CLASS, class);
  didl_add_tag (out, DIDL_CONTAINER_TITLE, title);

  buffer_appendf (out, "</%s>", DIDL_CONTAINER);
}

static int
cds_browse_metadata (struct action_event_t *event, struct buffer_t *out,
                     int index, int count, struct upnp_entry_t *entry,
                     char *filter)
{
  int result_count = 0, c = 0;
  char *fullpath = NULL;

  if (!entry)
    return -1;

  if (entry->child_count == -1) /* item : file */
  {
#ifdef HAVE_DLNA
    extern struct ushare_t *ut;
#endif /* HAVE_DLNA */
    
    char *protocol =
#ifdef HAVE_DLNA
      entry->dlna_profile ?
      dlna_write_protocol_info (DLNA_PROTOCOL_INFO_TYPE_HTTP,
                                DLNA_ORG_PLAY_SPEED_NORMAL,
                                DLNA_ORG_CONVERSION_NONE,
                                DLNA_ORG_OPERATION_RANGE,
                                ut->dlna_flags, entry->dlna_profile) :
#endif /* HAVE_DLNA */
      mime_get_protocol (entry->mime_type);
    
    didl_add_header (out);
#ifdef HAVE_DLNA
    entry->dlna_profile ?
      didl_add_item (out, entry->id, entry->parent
                     ? entry->parent->id : -1, "false",
                     dlna_profile_upnp_object_item (entry->dlna_profile),
                     entry->title,
                     protocol, entry->size,
                     entry->url, filter, NULL) :
#endif /* HAVE_DLNA */
      get_fullpath (entry, &fullpath);
      didl_add_item (out, entry->id, entry->parent
                     ? entry->parent->id : -1, "false",
                     entry->mime_type->mime_class, entry->title,
                     protocol, entry->size,
                     entry->url, filter, fullpath);

      if(fullpath != NULL)
        free(fullpath);

    didl_add_footer (out);
    free (protocol);
    
    for (c = index; c < MIN (index + count, entry->child_count); c++)
      result_count++;
  }
  else  /* container : directory */
  {
    didl_add_header (out);
    didl_add_container (out, entry->id, entry->parent
                        ? entry->parent->id : -1, entry->child_count,
                        "true", "true", entry->title,
                        entry->mime_type->mime_class);
    didl_add_footer (out);

    result_count = 1;
  }

  upnp_add_response (event, SERVICE_CDS_DIDL_RESULT, out->buf);
  upnp_add_response (event, SERVICE_CDS_DIDL_NUM_RETURNED, "1");
  upnp_add_response (event, SERVICE_CDS_DIDL_TOTAL_MATCH, "1");

  return result_count;
}

/* ========================================================================== */
static int cmp_class_u(const void *e1, const void *e2)
{
	struct upnp_entry_t **entry1 = (struct upnp_entry_t **)e1;
	struct upnp_entry_t **entry2 = (struct upnp_entry_t **)e2;

	return strcasecmp((*entry1)->mime_type->mime_class, (*entry2)->mime_type->mime_class);
}

static int cmp_class_d(const void *e1, const void *e2)
{
	return -cmp_class_u(e1, e2);
}

static int cmp_title_u(const void *e1, const void *e2)
{
	struct upnp_entry_t **entry1 = (struct upnp_entry_t **)e1;
	struct upnp_entry_t **entry2 = (struct upnp_entry_t **)e2;

	return strcasecmp((*entry1)->title, (*entry2)->title);
}

static int cmp_title_d(const void *e1, const void *e2)
{
	return -cmp_title_u(e1, e2);
}

static struct upnp_entry_t **cds_browse_sort(struct upnp_entry_t *childs[], int child_count, const char *sort_criteria)
{
	struct upnp_entry_t **chlist = NULL;
	char *sc;
	char *cp;
	int n;
	int folder_count;
	int item_count;
	int class_order = 1;
	int title_order = 1;

	if ((chlist = (struct upnp_entry_t **)malloc(sizeof(struct upnp_entry_t *) * (child_count + 1))) != NULL) {
		for (n = 0, folder_count = 0, item_count = 0; n < child_count; n++) {
			chlist[n] = childs[n];
			if (chlist[n]->child_count >= 0) 	folder_count++;
			else								item_count++;
		}
		chlist[n] = NULL;

		qsort(chlist, child_count, sizeof(struct upnp_entry_t *), cmp_class_u);		/* folders -> items */

		if (sort_criteria != NULL) {
			if ((sc = (char *)malloc(strlen(sort_criteria) + 1)) != NULL) {
				strcpy(sc, sort_criteria);

				cp = strtok(sc, ",");
				while (cp != NULL) {
					if (strstr(cp, "class") != NULL) {
						class_order = (*cp == '-')? 0 : 1;;
						if (!class_order) qsort(chlist, child_count, sizeof(struct upnp_entry_t *), cmp_class_d);
					}
					else if (strstr(cp, "title") != NULL) {
						title_order = (*cp == '-')? 0 : 1;;
						if (class_order) {
							qsort(&chlist[0], folder_count, sizeof(struct upnp_entry_t *), (title_order)? cmp_title_u: cmp_title_d);
							qsort(&chlist[folder_count], item_count, sizeof(struct upnp_entry_t *), (title_order)? cmp_title_u: cmp_title_d);
						}
						else
						{
							qsort(&chlist[0], item_count, sizeof(struct upnp_entry_t *), (title_order)? cmp_title_u: cmp_title_d);
							qsort(&chlist[item_count], folder_count, sizeof(struct upnp_entry_t *), (title_order)? cmp_title_u: cmp_title_d);
						}
					}
					cp = strtok(NULL, ",");
				}
				free(sc);
			}
		}
	}

	return chlist;
}
/* ========================================================================== */

static int
cds_browse_directchildren (struct action_event_t *event,
                           struct buffer_t *out, int index,
                           int count, struct upnp_entry_t *entry, char *filter,
                           const char *sort_criteria)
{
  struct upnp_entry_t **childs;
  struct upnp_entry_t **childs_sorted;
  int s, result_count = 0;
  char tmp[32];
  char *fullpath = NULL;

  if (entry->child_count == -1) /* item : file */
    return -1;

  didl_add_header (out);

  /* go to the child pointed out by index */
/* childs = entry->childs; */
  childs_sorted = cds_browse_sort(entry->childs, entry->child_count, sort_criteria);
  childs = childs_sorted;

  for (s = 0; s < index; s++)
    if (*childs)
      childs++;

  /* UPnP CDS compliance : If starting index = 0 and requested count = 0
     then all children must be returned */
  if (index == 0 && count == 0)
    count = entry->child_count;

  for (; *childs; childs++)
  {
    if (count == 0 || result_count < count)
      /* only fetch the requested count number or all entries if count = 0 */
    {
      if ((*childs)->child_count >= 0) /* container */
        didl_add_container (out, (*childs)->id, (*childs)->parent ?
                            (*childs)->parent->id : -1,
                            (*childs)->child_count, "true", NULL,
                            (*childs)->title,
                            (*childs)->mime_type->mime_class);
      else /* item */
      {
#ifdef HAVE_DLNA
        extern struct ushare_t *ut;
#endif /* HAVE_DLNA */

        char *protocol =
#ifdef HAVE_DLNA
          (*childs)->dlna_profile ?
          dlna_write_protocol_info (DLNA_PROTOCOL_INFO_TYPE_HTTP,
                                    DLNA_ORG_PLAY_SPEED_NORMAL,
                                    DLNA_ORG_CONVERSION_NONE,
                                    DLNA_ORG_OPERATION_RANGE,
                                    ut->dlna_flags, (*childs)->dlna_profile) :
#endif /* HAVE_DLNA */
          mime_get_protocol ((*childs)->mime_type);

#ifdef HAVE_DLNA
        (*childs)->dlna_profile ?
          didl_add_item (out, (*childs)->id,
                         (*childs)->parent ? (*childs)->parent->id : -1,
                         "true", dlna_profile_upnp_object_item ((*childs)->dlna_profile),
                         (*childs)->title, protocol,
                         (*childs)->size, (*childs)->url, filter, NULL) :
#endif /* HAVE_DLNA */
          get_fullpath ((*childs), &fullpath);
          didl_add_item (out, (*childs)->id,
                         (*childs)->parent ? (*childs)->parent->id : -1,
                         "true", (*childs)->mime_type->mime_class,
                         (*childs)->title, protocol,
                         (*childs)->size, (*childs)->url, filter, fullpath);
          if(fullpath != NULL)
             free(fullpath);

        free (protocol);
      }
      result_count++;
    }
  }

  free(childs_sorted);

  didl_add_footer (out);

  upnp_add_response (event, SERVICE_CDS_DIDL_RESULT, out->buf);
  sprintf (tmp, "%d", result_count);
  upnp_add_response (event, SERVICE_CDS_DIDL_NUM_RETURNED, tmp);
  sprintf (tmp, "%d", entry->child_count);
  upnp_add_response (event, SERVICE_CDS_DIDL_TOTAL_MATCH, tmp);

  return result_count;
}

static bool
cds_browse (struct action_event_t *event)
{
  extern struct ushare_t *ut;
  struct upnp_entry_t *entry = NULL;
  int result_count = 0, index, count, id/* , sort_criteria */;
  char *flag = NULL;
  char *filter = NULL;
  struct buffer_t *out = NULL;
  bool metadata;
  char *sort_criteria = NULL;

  if (!event)
    return false;

  /* Check for status */
  if (!event->status)
    return false;

  /* check if metadatas have been well inited */
  if (!ut->init)
    return false;

  /* Retrieve Browse arguments */
  index = upnp_get_ui4 (event->request, SERVICE_CDS_ARG_START_INDEX);
  count = upnp_get_ui4 (event->request, SERVICE_CDS_ARG_REQUEST_COUNT);
  id = upnp_get_ui4 (event->request, SERVICE_CDS_ARG_OBJECT_ID);
  flag = upnp_get_string (event->request, SERVICE_CDS_ARG_BROWSE_FLAG);
  filter = upnp_get_string (event->request, SERVICE_CDS_ARG_FILTER);
/* sort_criteria = upnp_get_ui4 (event->request, SERVICE_CDS_ARG_SORT_CRIT); */
  sort_criteria = upnp_get_string (event->request, SERVICE_CDS_ARG_SORT_CRIT);

  if (!flag || !filter)
    return false;

  /* Check arguments validity */
  if (!strcmp (flag, SERVICE_CDS_BROWSE_METADATA))
    {
      if (index)
      {
        free (flag);
        free (sort_criteria);
        return false;
      }
      metadata = true;
    }
  else if (!strcmp (flag, SERVICE_CDS_BROWSE_CHILDREN))
    metadata = false;
  else
  {
    free (sort_criteria);
    free (flag);
    return false;
  }
  free (flag);

  entry = upnp_get_entry (ut, id);
  if (!entry && (id < ut->starting_id))
#if 0
    entry = upnp_get_entry (ut, ut->starting_id);
#else
    entry = ut->root_entry;
#endif

  if (!entry)
  {
    free (sort_criteria);
    free (filter);
    return false;
  }

  out = buffer_new ();
  if (!out)
  {
    free (sort_criteria);
    free (filter);
    return false;
  }

  if (metadata)
    result_count =
      cds_browse_metadata (event, out, index, count, entry, filter);
  else
    result_count =
      cds_browse_directchildren (event, out, index, count, entry, filter, sort_criteria);
  free (sort_criteria);
  free (filter);

  if (result_count < 0)
  {
    buffer_free (out);
    return false;
  }

  buffer_free (out);
  upnp_add_response (event, SERVICE_CDS_DIDL_UPDATE_ID,
                     SERVICE_CDS_ROOT_OBJECT_ID);

  return event->status;
}

static bool
matches_search (char *search_criteria, struct upnp_entry_t *entry)
{
  char keyword[256] = SEARCH_OBJECT_KEYWORD;
  bool derived_from = false, protocol_contains = false, result = false;
  char *quote_closed = NULL, *and_clause = NULL;
#ifdef HAVE_DLNA
  extern struct ushare_t *ut;
#endif /* HAVE_DLNA */
  char *protocol =
#ifdef HAVE_DLNA
    entry->dlna_profile ?
    dlna_write_protocol_info (DLNA_PROTOCOL_INFO_TYPE_HTTP,
                              DLNA_ORG_PLAY_SPEED_NORMAL,
                              DLNA_ORG_CONVERSION_NONE,
                              DLNA_ORG_OPERATION_RANGE,
                              ut->dlna_flags, entry->dlna_profile) :
#endif /* HAVE_DLNA */
    mime_get_protocol (entry->mime_type);
  
  if (!strncmp (search_criteria, SEARCH_CLASS_MATCH_KEYWORD,
                strlen (SEARCH_CLASS_MATCH_KEYWORD)))
  {
    strncpy (keyword, search_criteria
             + strlen (SEARCH_CLASS_MATCH_KEYWORD), sizeof (keyword));
    quote_closed = strchr (keyword, '"');

    if (quote_closed)
      *quote_closed = '\0';
  }
  else if (!strncmp (search_criteria, SEARCH_CLASS_DERIVED_KEYWORD,
                     strlen (SEARCH_CLASS_DERIVED_KEYWORD)))
  {
    derived_from = true;
    strncpy (keyword, search_criteria
             + strlen (SEARCH_CLASS_DERIVED_KEYWORD), sizeof (keyword));
    quote_closed = strchr (keyword, '"');

    if (quote_closed)
      *quote_closed = '\0';
  }
  else if (!strncmp (search_criteria, SEARCH_PROTOCOL_CONTAINS_KEYWORD,
                     strlen (SEARCH_PROTOCOL_CONTAINS_KEYWORD)))
  {
    protocol_contains = true;
    strncpy (keyword, search_criteria
             + strlen (SEARCH_PROTOCOL_CONTAINS_KEYWORD), sizeof (keyword));
    quote_closed = strchr (keyword, '"');

    if (quote_closed)
      *quote_closed = '\0';
  }

  if (derived_from && entry->mime_type
      && !strncmp (entry->mime_type->mime_class, keyword, strlen (keyword)))
    result = true;
  else if (protocol_contains && strstr (protocol, keyword))
    result = true;
  else if (entry->mime_type &&
           !strcmp (entry->mime_type->mime_class, keyword))
    result = true;
  free (protocol);
  
  and_clause = strstr (search_criteria, SEARCH_AND);
  if (and_clause)
    return (result &&
            matches_search (and_clause + strlen (SEARCH_AND) -1, entry));

  return result;
}

#if 1
static int
cds_search_directchildren_recursive (struct buffer_t *out,
                                     struct upnp_entry_t *entry, char *filter,
                                     char *search_criteria,
                                     struct upnp_entry_t **elist)
{
	struct upnp_entry_t **childs;
	int result_count = 0;

	if (entry->child_count == -1) /* item : file */
		return -1;

	/* go to the first child */
	childs = entry->childs;

	for (; *childs; childs++) {
		if ((*childs)->child_count >= 0) {	/* container */
			int new_count;
			new_count = cds_search_directchildren_recursive(
			  out, (*childs), filter, search_criteria, elist);
			if (new_count > 0) {
				result_count += new_count;
				if (elist != NULL) elist += new_count;
			}
		}
		else {	/* item */
			if (matches_search (search_criteria, *childs)) {
				result_count++;
				if (elist != NULL) *elist++ = *childs;
			}
		}
	}

	return result_count;
}
#else
static int
cds_search_directchildren_recursive (struct buffer_t *out, int count,
                                     struct upnp_entry_t *entry, char *filter,
                                     char *search_criteria)
{
  struct upnp_entry_t **childs;
  int result_count = 0;

  if (entry->child_count == -1) /* item : file */
    return -1;

  /* go to the first child */
  childs = entry->childs;

  for (; *childs; childs++)
  {
    if (count == 0 || result_count < count)
      /* only fetch the requested count number or all entries if count = 0 */
    {
      if ((*childs)->child_count >= 0) /* container */
      {
        int new_count;
        new_count = cds_search_directchildren_recursive
          (out, (count == 0) ? 0 : (count - result_count),
           (*childs), filter, search_criteria);
        result_count += new_count;
      }
      else /* item */
      {
        if (matches_search (search_criteria, *childs))
        {
#ifdef HAVE_DLNA
          extern struct ushare_t *ut;
#endif /* HAVE_DLNA */
          char *protocol =
#ifdef HAVE_DLNA
            (*childs)->dlna_profile ?
            dlna_write_protocol_info(DLNA_PROTOCOL_INFO_TYPE_HTTP,
                                     DLNA_ORG_PLAY_SPEED_NORMAL,
                                     DLNA_ORG_CONVERSION_NONE,
                                     DLNA_ORG_OPERATION_RANGE,
                                     ut->dlna_flags, (*childs)->dlna_profile):
#endif /* HAVE_DLNA */
            mime_get_protocol ((*childs)->mime_type);

#ifdef HAVE_DLNA
          (*childs)->dlna_profile ?
            didl_add_item (out, (*childs)->id,
                           (*childs)->parent ? (*childs)->parent->id : -1,
                           "true", dlna_profile_upnp_object_item ((*childs)->dlna_profile),
                           (*childs)->title, protocol,
                           (*childs)->size, (*childs)->url, filter) :
#endif /* HAVE_DLNA */
            didl_add_item (out, (*childs)->id,
                           (*childs)->parent ? (*childs)->parent->id : -1,
                           "true", (*childs)->mime_type->mime_class,
                           (*childs)->title, protocol,
                           (*childs)->size, (*childs)->url, filter);
          free (protocol);
          result_count++;
        }
      }
    }
  }

  return result_count;
}
#endif

#if 1
static int
cds_search_directchildren (struct action_event_t *event,
                           struct buffer_t *out, int index,
                           int count, struct upnp_entry_t *entry,
                           char *filter, char *search_criteria,
                           char *sort_criteria)
{
	struct upnp_entry_t **childs;
	int s, result_count = 0;
	char tmp[32];

	struct upnp_entry_t **found_list;
	struct upnp_entry_t **ep;
	char *fullpath = NULL;

	if (entry->child_count == -1) /* item : file */
		return -1;

	didl_add_header (out);

	/* go to the child pointed out by index */
	childs = entry->childs;

	/* step 1: count items (pass 1) */
	for (; *childs; childs++) {
		if ((*childs)->child_count >= 0) {	/* container */
			int new_count;
			new_count = cds_search_directchildren_recursive(
			  out, (*childs), filter, search_criteria, NULL);
			if (new_count > 0) result_count += new_count;
		}
		else {	/* item */
			if (matches_search (search_criteria, *childs)) result_count++;
		}
	}

	/* step 2: construct item list (pass 2) */
	if ((found_list = (struct upnp_entry_t **)malloc(sizeof(struct upnp_entry_t *) * (result_count + 1))) != NULL) {
		for (s = 0; s < (result_count + 1); s++) found_list[s] = NULL;	/* clear */

		childs = entry->childs;
		for (ep = found_list; *childs; childs++) {
			if ((*childs)->child_count >= 0) {	/* container */
				int new_count;
				new_count = cds_search_directchildren_recursive(
				  out, (*childs), filter, search_criteria, ep);
				if (new_count > 0) ep += new_count;
			}
			else {	/* item */
				if (matches_search(search_criteria, *childs)) *ep++ = *childs;
			}
		}

		/* step 3: sort */
		if (sort_criteria != NULL) {
			char *sc;
			char *cp;

			if ((sc = (char *)malloc(strlen(sort_criteria) + 1)) != NULL) {
				strcpy(sc, sort_criteria);
				cp = strtok(sc, ",");
				while (cp != NULL) {
					if (strstr(cp, "class") != NULL) {
						qsort(found_list, result_count, sizeof(struct upnp_entry_t *), (*cp == '-')? cmp_class_d: cmp_class_u);
					}
					else if (strstr(cp, "title") != NULL) {
						qsort(found_list, result_count, sizeof(struct upnp_entry_t *), (*cp == '-')? cmp_title_d: cmp_title_u);
					}
					cp = strtok(NULL, ",");
				}
				free(sc);
			}
		}

		/* step 4: make didl */
		/* UPnP CDS compliance : If starting index = 0 and requested count = 0
		 then all children must be returned */
		if (index == 0 && count == 0) count = result_count;

		for (s = 0, childs = found_list; s < index; s++, childs++);
		for (; *childs && count > 0; childs++, count--) {
#ifdef HAVE_DLNA
			extern struct ushare_t *ut;
#endif /* HAVE_DLNA */
			char *protocol =
#ifdef HAVE_DLNA
			(*childs)->dlna_profile ?
			dlna_write_protocol_info(DLNA_PROTOCOL_INFO_TYPE_HTTP,
			                         DLNA_ORG_PLAY_SPEED_NORMAL,
			                         DLNA_ORG_CONVERSION_NONE,
			                         DLNA_ORG_OPERATION_RANGE,
			                         ut->dlna_flags, (*childs)->dlna_profile):
#endif /* HAVE_DLNA */
			mime_get_protocol ((*childs)->mime_type);

#ifdef HAVE_DLNA
			(*childs)->dlna_profile ?
			didl_add_item (out, (*childs)->id,
			               (*childs)->parent ? (*childs)->parent->id : -1,
			               "true", dlna_profile_upnp_object_item ((*childs)->dlna_profile),
			               (*childs)->title, protocol,
			               (*childs)->size, (*childs)->url, filter, NULL) :
#endif /* HAVE_DLNA */
            get_fullpath ((*childs), &fullpath);
			didl_add_item (out, (*childs)->id,
			               (*childs)->parent ? (*childs)->parent->id : -1,
			               "true", (*childs)->mime_type->mime_class,
			               (*childs)->title, protocol,
			               (*childs)->size, (*childs)->url, filter, fullpath);
			if(fullpath != NULL)
			  free(fullpath);

			free (protocol);
		}

		free(found_list);
	}
	else
		result_count = 0;

	didl_add_footer (out);

	upnp_add_response (event, SERVICE_CDS_DIDL_RESULT, out->buf);

	sprintf (tmp, "%d", result_count);
	upnp_add_response (event, SERVICE_CDS_DIDL_NUM_RETURNED, tmp);
	sprintf (tmp, "%d", result_count);
	upnp_add_response (event, SERVICE_CDS_DIDL_TOTAL_MATCH, tmp);

	return result_count;
}
#else
static int
cds_search_directchildren (struct action_event_t *event,
                           struct buffer_t *out, int index,
                           int count, struct upnp_entry_t *entry,
                           char *filter, char *search_criteria,
                           char *sort_criteria)
{
  struct upnp_entry_t **childs;
  int s, result_count = 0;
  char tmp[32];

  struct upnp_entry_t **found_list;
  stcuct upnp_entry_t **ep;

  index = 0;

  if (entry->child_count == -1) /* item : file */
    return -1;

  didl_add_header (out);

  /* go to the child pointed out by index */
  childs = entry->childs;
  for (s = 0; s < index; s++)
    if (*childs)
      childs++;

  /* UPnP CDS compliance : If starting index = 0 and requested count = 0
     then all children must be returned */
  if (index == 0 && count == 0)
    count = entry->child_count;

  for (; *childs; childs++)
  {
    if (count == 0 || result_count < count)
      /* only fetch the requested count number or all entries if count = 0 */
    {
      if ((*childs)->child_count >= 0) /* container */
      {
        int new_count;
        new_count = cds_search_directchildren_recursive
          (out, (count == 0) ? 0 : (count - result_count),
           (*childs), filter, search_criteria);
        result_count += new_count;
      }
      else /* item */
      {
        if (matches_search (search_criteria, *childs))
        {
#ifdef HAVE_DLNA
          extern struct ushare_t *ut;
#endif /* HAVE_DLNA */
          char *protocol =
#ifdef HAVE_DLNA
            (*childs)->dlna_profile ?
            dlna_write_protocol_info(DLNA_PROTOCOL_INFO_TYPE_HTTP,
                                     DLNA_ORG_PLAY_SPEED_NORMAL,
                                     DLNA_ORG_CONVERSION_NONE,
                                     DLNA_ORG_OPERATION_RANGE,
                                     ut->dlna_flags, (*childs)->dlna_profile):
#endif /* HAVE_DLNA */
            mime_get_protocol ((*childs)->mime_type);

#ifdef HAVE_DLNA
          (*childs)->dlna_profile ?
            didl_add_item (out, (*childs)->id,
                           (*childs)->parent ? (*childs)->parent->id : -1,
                           "true", dlna_profile_upnp_object_item ((*childs)->dlna_profile),
                           (*childs)->title, protocol,
                           (*childs)->size, (*childs)->url, filter) :
#endif /* HAVE_DLNA */
            didl_add_item (out, (*childs)->id,
                           (*childs)->parent ? (*childs)->parent->id : -1,
                           "true", (*childs)->mime_type->mime_class,
                           (*childs)->title, protocol,
                           (*childs)->size, (*childs)->url, filter);
          free (protocol);
          result_count++;
        }
      }
    }
  }

  didl_add_footer (out);

  upnp_add_response (event, SERVICE_CDS_DIDL_RESULT, out->buf);

  sprintf (tmp, "%d", result_count);
  upnp_add_response (event, SERVICE_CDS_DIDL_NUM_RETURNED, tmp);
  sprintf (tmp, "%d", result_count);
  upnp_add_response (event, SERVICE_CDS_DIDL_TOTAL_MATCH, tmp);

  return result_count;
}
#endif

static bool
cds_search (struct action_event_t *event)
{
  extern struct ushare_t *ut;
  struct upnp_entry_t *entry = NULL;
  int result_count = 0, index, count, id/*, sort_criteria*/;
  char *search_criteria = NULL;
  char *filter = NULL;
  struct buffer_t *out = NULL;
  char *sort_criteria = NULL;

  if (!event)
    return false;

  /* Check for status */
  if (!event->status)
    return false;

  /* check if metadatas have been well inited */
  if (!ut->init)
    return false;

  /* Retrieve Browse arguments */
  index = upnp_get_ui4 (event->request, SERVICE_CDS_ARG_START_INDEX);
  count = upnp_get_ui4 (event->request, SERVICE_CDS_ARG_REQUEST_COUNT);
  id = upnp_get_ui4 (event->request, SERVICE_CDS_ARG_OBJECT_ID);

  search_criteria = upnp_get_string (event->request,
                                     SERVICE_CDS_ARG_SEARCH_CRIT);
  filter = upnp_get_string (event->request, SERVICE_CDS_ARG_FILTER);
/* sort_criteria = upnp_get_ui4 (event->request, SERVICE_CDS_ARG_SORT_CRIT); */
  sort_criteria = upnp_get_string (event->request, SERVICE_CDS_ARG_SORT_CRIT);

fprintf(stdout, "%s.%d search %d %d %d %s %s\n", __FILE__, __LINE__, index, count, id, search_criteria, sort_criteria);

  if (!search_criteria || !filter)
  {
    free(sort_criteria);
    free(search_criteria);
    free(filter);
    return false;
  }

  entry = upnp_get_entry (ut, id);

  if (!entry && (id < ut->starting_id))
#if 0
    entry = upnp_get_entry (ut, ut->starting_id);
#else
    entry = ut->root_entry;
#endif

  if (!entry)
  {
    free(sort_criteria);
    free(search_criteria);
    free(filter);
    return false;
  }

  out = buffer_new ();
  if (!out)
  {
    free(sort_criteria);
    free(search_criteria);
    free(filter);
    return false;
  }

  result_count =
    cds_search_directchildren (event, out, index, count, entry,
                               filter, search_criteria, sort_criteria);

  if (result_count < 0)
  {
    buffer_free (out);
    free(sort_criteria);
    free(search_criteria);
    free(filter);
    return false;
  }

  buffer_free (out);
  upnp_add_response (event, SERVICE_CDS_DIDL_UPDATE_ID,
                     SERVICE_CDS_ROOT_OBJECT_ID);

  free (sort_criteria);
  free (search_criteria);
  free (filter);

  return event->status;
}

/* List of UPnP ContentDirectory Service actions */
struct service_action_t cds_service_actions[] = {
  { SERVICE_CDS_ACTION_SEARCH_CAPS, cds_get_search_capabilities },
  { SERVICE_CDS_ACTION_SORT_CAPS, cds_get_sort_capabilities },
  { SERVICE_CDS_ACTION_UPDATE_ID, cds_get_system_update_id },
  { SERVICE_CDS_ACTION_BROWSE, cds_browse },
  { SERVICE_CDS_ACTION_SEARCH, cds_search },
  { NULL, NULL }
};
