/**
 * @file
 *
 * debug message(log message)
 *
 *
 * Copyright (C) 2009 by silex technology, Inc.
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License 
 * any later version.
**/
#ifndef _SX_DEBUG_H
#define _SX_DEBUG_H

#define SXLOGLEVEL 0

#if 0
#define DEFSYSLOG(level) syslog(level)
#else
void sxsetloglebel(int);
BOOL sxsyslog( const char*, ... );
#define DEFSYSLOG(level,fmt) \
{ \
	syslog(level,"%s:%s(%d)",__FILE__,__FUNCTION__,__LINE__); \
	sxsetloglebel(level); \
	sxsyslog fmt; \
}
#endif


#if (SXLOGLEVEL>=0)
#define DEBUG0(fmt) DEFSYSLOG(LOG_ERR, fmt) 
#else
#define DEBUG0(fmt) {}
#endif

#if (SXLOGLEVEL>=1)
#define DEBUG1(fmt) DEFSYSLOG(LOG_WARNING,fmt) 
#else
#define DEBUG1(fmt) {}
#endif

#if (SXLOGLEVEL>=2)
#define DEBUG2(fmt) DEFSYSLOG(LOG_WARNING,fmt)
#else
#define DEBUG2(fmt) {}
#endif

#if (SXLOGLEVEL>=3)
#define DEBUG3(fmt) DEFSYSLOG(LOG_NOTICE,fmt)
#else
#define DEBUG3(fmt) {}
#endif

#if (SXLOGLEVEL>=4)
#define DEBUG4(fmt) DEFSYSLOG(LOG_INFO,fmt)
#else
#define DEBUG4(fmt) {}
#endif

#if (SXLOGLEVEL>=5)
#define DEBUG5(fmt) DEFSYSLOG(LOG_INFO,fmt)
#else
#define DEBUG5(fmt) {}
#endif

#if (SXLOGLEVEL>=6)
#define DEBUG6(fmt) DEFSYSLOG(LOG_INFO,fmt)
#else
#define DEBUG6(fmt) {}
#endif

#if (SXLOGLEVEL>=7)
#define DEBUG7(fmt) DEFSYSLOG(LOG_DEBUG,fmt)
#else
#define DEBUG7(fmt) {}
#endif

#if (SXLOGLEVEL>=8)
#define DEBUG8(fmt) DEFSYSLOG(LOG_DEBUG,fmt)
#else
#define DEBUG8(fmt) {}
#endif

#if (SXLOGLEVEL>=9)
#define DEBUG9(fmt) DEFSYSLOG(LOG_DEBUG,fmt)
#else
#define DEBUG9(fmt) {}
#endif

#if (SXLOGLEVEL>=10)
#define DEBUG10(fmt) DEFSYSLOG(LOG_DEBUG,fmt)
#else
#define DEBUG10(fmt) {}
#endif

#if 1
#define DEBUG(level, body ) \
  (void)( ((level) <= MAX_DEBUG_LEVEL) && \
           ((DEBUGLEVEL_CLASS[ DBGC_CLASS ] >= (level))||  \
           (!DEBUGLEVEL_CLASS_ISSET[ DBGC_CLASS ] && \
            DEBUGLEVEL_CLASS[ DBGC_ALL   ] >= (level))  ) \
       && (dbghdr( level, __FILE__, FUNCTION_MACRO, (__LINE__) )) \
       && (dbgtext body) )
#else
#define DEBUG(level, body ) DEFSYSLOG(LOG_DEBUG, body)
#endif

#endif
