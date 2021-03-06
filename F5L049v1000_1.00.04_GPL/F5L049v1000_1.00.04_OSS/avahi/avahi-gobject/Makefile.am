# $Id: Makefile.am 1566 2007-10-28 18:41:43Z sjoerd $
#
# This file is part of avahi.
#
# avahi is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# avahi is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
# License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with avahi; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.

AM_CFLAGS=-I$(top_srcdir)

# This cool debug trap works on i386/gcc only
AM_CFLAGS+='-DDEBUG_TRAP=__asm__("int $$3")'

if HAVE_GOBJECT

avahigobjectincludedir=$(includedir)/avahi-gobject

avahigobjectinclude_HEADERS = \
	ga-client.h \
	ga-entry-group.h \
	ga-enums.h \
	ga-error.h \
	ga-record-browser.h \
	ga-service-browser.h \
	ga-service-resolver.h

lib_LTLIBRARIES = \
	libavahi-gobject.la

BUILT_SOURCES = \
	signals-marshal.list \
	signals-marshal.h \
	signals-marshal.c \
	ga-client-enumtypes.h \
	ga-client-enumtypes.c  \
	ga-entry-group-enumtypes.h \
	ga-entry-group-enumtypes.c  \
	ga-enums-enumtypes.h \
	ga-enums-enumtypes.c

CORE_SOURCES = \
	ga-client.c ga-client.h \
	ga-entry-group.c ga-entry-group.h \
	ga-enums.h \
	ga-error.c ga-error.h \
	ga-record-browser.c ga-record-browser.h \
	ga-service-browser.c ga-service-browser.h \
	ga-service-resolver.c ga-service-resolver.h

libavahi_gobject_la_SOURCES = \
	$(CORE_SOURCES) \
	$(BUILT_SOURCES)

libavahi_gobject_la_CFLAGS = $(AM_CFLAGS) $(GOBJECT_CFLAGS)
libavahi_gobject_la_LIBADD = $(AM_LDADD) ../avahi-common/libavahi-common.la ../avahi-client/libavahi-client.la ../avahi-glib/libavahi-glib.la $(GOJECT_LIBS)
libavahi_gobject_la_LDFLAGS = $(AM_LDFLAGS) -export-dynamic -version-info $(LIBAVAHI_GOBJECT_VERSION_INFO)

# correctly clean the generated source files
CLEANFILES = $(BUILT_SOURCES)

dist-hook:
	$(shell for x in $(BUILT_SOURCES); do rm -f $(distdir)/$$x ; done)

signals-marshal.list: $(CORE_SOURCES) Makefile.am
	( cd $(srcdir) && \
	sed -n -e 's/.*ga_signals_marshal_\([A-Z]*__[A-Z_]*\).*/\1/p' \
		$(CORE_SOURCES) ) \
		| sed -e 's/__/:/' -e 'y/_/,/' | sort -u > $@.tmp
	if cmp -s $@.tmp $@; then \
		rm $@.tmp; \
	else \
		mv $@.tmp $@; \
	fi

signals-marshal.h: signals-marshal.list
	glib-genmarshal --header --prefix=ga_signals_marshal $< > $@

signals-marshal.c: signals-marshal.list
	glib-genmarshal --body --prefix=ga_signals_marshal $< > $@


# rules for making the glib enum objects
%-enumtypes.h: %.h Makefile.in
	glib-mkenums \
	--fhead "#ifndef __$(shell echo $* | tr [:lower:]- [:upper:]_)_ENUM_TYPES_H__\n#define __$(shell echo $* | tr [:lower:]- [:upper:]_)_ENUM_TYPES_H__\n\n#include <glib-object.h>\n\nG_BEGIN_DECLS\n" \
	--fprod "/* enumerations from \"@filename@\" */\n" \
	--vhead "GType @enum_name@_get_type (void);\n#define $(shell echo $* | tr [:lower:]- [:upper:]_ | sed 's/_.*//')_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n"         \
	--ftail "G_END_DECLS\n\n#endif /* __$(shell echo $* | tr [:lower:]- [:upper:]_)_ENUM_TYPES_H__ */" \
	$< > $@

%-enumtypes.c: %.h Makefile.in
	glib-mkenums \
	--fhead "#include <$*.h>" \
	--fprod "\n/* enumerations from \"@filename@\" */" \
	--vhead "GType\n@enum_name@_get_type (void)\n{\n  static GType etype = 0;\n  if (etype == 0) {\n    static const G@Type@Value values[] = {"     \
	--vprod "      { @VALUENAME@, \"@VALUENAME@\", \"@VALUENAME@\" }," \
	--vtail "      { 0, NULL, NULL }\n    };\n    etype = g_@type@_register_static (\"@EnumName@\", values);\n  }\n  return etype;\n}\n" \
	$< > $@

endif

indent:
	indent -brf -nbbo -nbc -ip0 -cs -nbfde -npsl -br -brs -bap -i4 -bs -cdw -ce -npcs -hnl -cli4 -nut -ci8 ga-*.[ch]
