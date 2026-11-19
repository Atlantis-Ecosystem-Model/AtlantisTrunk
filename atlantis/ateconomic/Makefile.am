
include $(top_srcdir)/PreRules.am
lib_LIBRARIES=libateconomic.a

# Headers for the external FishMIP-OSP toy-model plug-in (see FISHMIP_OSP_DIR
# in PreRules.am), needed by atFishMIP_OSP_PluginAdapter.cpp.
INCLUDES += -I$(FISHMIP_OSP_DIR)/cpp/include

# atFishMIP_OSP_PluginAdapter.cpp uses std::optional (C++17). Without an
# explicit -std, this toolchain's default C++ dialect is too old for it.
# (Per-target *_CXXFLAGS isn't honoured for plain lib_LIBRARIES in this
# Automake version, so this is set directory-wide; it has no effect on the
# .c sources here, which compile via the separate AM_CFLAGS/CFLAGS path.)
AM_CXXFLAGS = -std=c++17

libateconomic_adir=$(includedir)/ateconomic

libateconomic_a_SOURCES = ateconindicator.c ateconomic.c ateconomicsetup.c \
    ateconresponse.c ateconts.c atquota.c ateconeffort.c \
    ateconio.c ateconParamIO.c  \
    ateconomic.h ateconomicsetup.h atEconomicLib.h \
    ateffortquota.c atEconomicAnnual.c atEconomicUtil.c \
    atFishMIP_OSP_Bridge.c atFishMIP_OSP_PluginAdapter.cpp
    
#libeconomic_a_HEADERS= ateconomic.h atEconomicModule.h ateconomicsetup.h ateconParamIO.h atFishMIP_OSP_Bridge.h atFishMIP_OSP_PluginAdapter.h

# overide the -O2 flag
CFLAGS=

#WARN += -Wno-unused-parameter
#WARN += -Werror
