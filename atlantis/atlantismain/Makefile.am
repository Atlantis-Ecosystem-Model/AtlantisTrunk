include $(top_srcdir)/PreRules.am

bin_PROGRAMS   = atlantisMerged
atlantisMergeddir=$(includedir)/atlantismain
atlantisMerged_SOURCES = atlantismain.c atlantisboxmodel.h

atlantisMerged_LDADD = -latlantisutil -latassess -latphysics -latecology -lateconomic -latmanage -latharvest \
    -latimplementation -latConvertAtlantis  -lsjwlib -latSS3Link \
    -lfishmip_osp_plugin_models

SVNWCPATH = -D'ATLANTIS_WCPATH="$(shell svn info  ../| grep URL | sed  's/URL: //g')"'
SVNWCDATE = -D'ATLANTIS_WCDATE="$(shell svn info  ../| grep "Last Changed Date:" | sed  's/Last Changed Date: //g')"'
#SVNDEV = -D'ATLANTIS_REVISION="$(shell svn info  ../| grep "Last Changed Rev:" | sed  's/Last Changed Rev: //g')"'

SVNDEV = -D'ATLANTIS_REVISION="$(shell svnversion -n .)"'

    
atlantismain.o:   .FORCE

.FORCE:

LIBDIRS = -L$(top_srcdir)/atassess -L$(top_srcdir)/atecology \
	-L$(top_srcdir)/ateconomic -L$(top_srcdir)/atlantisUtil -L$(top_srcdir)/atlantismain \
	-L$(top_srcdir)/atmanage -L$(top_srcdir)/atharvest -L$(top_srcdir)/atimplementation \
	-L$(top_srcdir)/atphysics -L$(top_srcdir)/ConvertAtlantis \
	-L$(top_srcdir)/sjwlib -L$(top_srcdir)/atSS3Link \
	-L$(FISHMIP_OSP_DIR)/cpp/build

atlantisMerged_DEPENDENCIES =  $(top_srcdir)/atassess/libatassess.a  $(top_srcdir)/atphysics/libatphysics.a \
     $(top_srcdir)/atecology/libatecology.a $(top_srcdir)/ateconomic/libateconomic.a \
     $(top_srcdir)/atharvest/libatharvest.a $(top_srcdir)/atimplementation/libatimplementation.a \
     $(top_srcdir)/atmanage/libatmanage.a \
     $(top_srcdir)/ConvertAtlantis/libatConvertAtlantis.a \
     $(top_srcdir)/atlantisUtil/libatlantisutil.a $(top_srcdir)/sjwlib/libsjwlib.a $(top_srcdir)/atSS3Link/libatSS3Link.a
# NOTE: libfishmip_osp_plugin_models.a is deliberately NOT listed here. Automake
# splices atlantisMerged_DEPENDENCIES into the target's prerequisite list, and
# GNU Make expands prerequisite lists immediately while parsing the Makefile —
# before FISHMIP_OSP_DIR's "?=" default (relocated by Automake to the bottom
# of the generated Makefile, since it doesn't recognise that variable) has
# been assigned. That silently expands to an empty string here, breaking the
# build ("No rule to make target `/cpp/build/lib...a'"). $(FISHMIP_OSP_DIR) is
# still safe to use inside LIBDIRS/LDFLAGS below, since those are only ever
# expanded inside the link recipe (deferred, not immediate). Practical upshot:
# if you rebuild the toy-model library, force a relink here too, e.g. by
# running `touch atlantismain.c` before `make`.

atlantisMerged_LDFLAGS =  $(LIBDIRS) $(shell pkg-config --libs libxml-2.0)

# libateconomic.a now contains atFishMIP_OSP_PluginAdapter.cpp (C++), so the
# final link must be driven by the C++ compiler/linker to pull in the C++
# runtime (libstdc++/libc++) and correctly handle C++ symbols such as
# operator new/delete, exceptions, and STL statics. atlantismain.c itself
# stays plain C; only this final link step changes.
atlantisMerged_LINK = $(CXX) $(AM_CFLAGS) $(CFLAGS) $(atlantisMerged_LDFLAGS) $(LDFLAGS) -o $@

atlantisMerged_INCLUDES = 	-I$(top_srcdir)/atassess/include -I$(top_srcdir)/atecology/include \
	-I$(top_srcdir)/ateconomic/include -I$(top_srcdir)/atlantisUtil/include -I$(top_srcdir)/atlantismain/include \
	-I$(top_srcdir)/atharvest/include -I$(top_srcdir)/atimplementation/include \
	-I$(top_srcdir)/atmanage/include -I$(top_srcdir)/atphysics/include -I$(top_srcdir)/ConvertAtlantis/include \
	-I$(top_srcdir)/sjwlib/include -I$(top_srcdir)/atSS3Link/includee \
	-I. -I$(top_srcdir)/atlantismain

if BROKER_LINK_ENABLED
LIBDIRS += -L$(top_srcdir)/atbrokerlink
atlantisMerged_LDADD += -lbrokerlink -lzmq -lprotobuf-c
atlantisMerged_INCLUDES += -I$(top_srcdir)/abrokerlink/include
atlantisMerged_DEPENDENCIES +=  $(top_srcdir)/atbrokerlink/libbrokerlink.a
endif

if LINK_ENABLED
LIBDIRS += -L$(top_srcdir)/atbrokerlink
atlantisMerged_LDADD += -latlink -lcsoap -lnanohttp
atlantisMerged_INCLUDES += -I$(top_srcdir)/abrokerlink/include
atlantisMerged_DEPENDENCIES +=  $(top_srcdir)/atbrokerlink/libbrokerlink.a
endif

if CLAM_LINK_ENABLED
LIBDIRS += -L$(top_srcdir)/atCLAMLink
atlantisMerged_LDADD += -latCLAMLink
atlantisMerged_INCLUDES += -I$(top_srcdir)/atCLAMLink/include
atlantisMerged_DEPENDENCIES +=  $(top_srcdir)/atCLAMLink/libatCLAMLink.a
endif


#if SS3_LINK_ENABLED
#LIBDIRS += -L$(top_srcdir)/atSS3Link
#atlantisMerged_LDADD += -latSS3Link
#atlantisMerged_INCLUDES += -I$(top_srcdir)/atSS3Link/include
#atlantisMerged_DEPENDENCIES +=  $(top_srcdir)/atSS3Link/libatSS3Link.a
#endif

library_includedir=$(includedir)/$(PACKAGE_NAME)-$(PACKAGE_VERSION)/$(PACKAGE_NAME)
library_include_HEADERS = include/atlantisboxmodel.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = atlantisMerged.pc



# overide the -O2 flag
CFLAGS= $(SVNWCPATH) $(SVNWCDATE) $(SVNDEV)

#WARN += -Wno-unused-parameter
#WARN += -Werror



