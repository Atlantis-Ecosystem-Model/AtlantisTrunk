#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.64])
AC_INIT([Atlantis],[3.0],[beth.fulton@csiro.au])
# AC_INIT([Atlantis],[3.0],[bec.gorton@csiro.au])

AC_CONFIG_SRCDIR([atlantismain/include/atlantisboxmodel.h])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

dnl -----------------------------------------------
dnl Package name and version number (user defined)
dnl -----------------------------------------------

GENERIC_LIBRARY_NAME=atlantis

#release versioning
GENERIC_MAJOR_VERSION=2
GENERIC_MINOR_VERSION=0
GENERIC_MICRO_VERSION=0



#API version (often = GENERIC_MAJOR_VERSION.GENERIC_MINOR_VERSION)
GENERIC_API_VERSION=2.0
AC_SUBST(GENERIC_API_VERSION)


#shared library versioning
GENERIC_LIBRARY_VERSION=1:2:0
#                       | | |
#                +------+ | +---+
#                |        |     |
#             current:revision:age
#                |        |     |
#                |        |     +- increment if interfaces have been added
#                |        |        set to zero if interfaces have been removed
#                                  or changed
#                |        +- increment if source code has changed
#                |           set to zero if current is incremented
#                +- increment if interfaces have been added, removed or changed


dnl --------------------------------
dnl Package name and version number
dnl --------------------------------

AC_SUBST(GENERIC_LIBRARY_VERSION)

PACKAGE=$GENERIC_LIBRARY_NAME
AC_SUBST(GENERIC_LIBRARY_NAME)

GENERIC_VERSION=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION.$GENERIC_MICRO_VERSION
GENERIC_RELEASE=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION
AC_SUBST(GENERIC_RELEASE)
AC_SUBST(GENERIC_VERSION)

VERSION=$GENERIC_VERSION

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB
#AM_PROG_CC_C_O
CFLAGS=-g
# Checks for libraries.
# FIXME: Replace `main' with a function in `-lcunit':
#AC_CHECK_LIB([cunit], [CU_initialize_registry], [],[
#           echo "cunit library is required for this program. Install package libcunit1-dev"
#           exit -1])

# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [abs], [],[
           echo "Maths library is required for Atlantis"
           exit -1])




# FIXME: Replace `main' with a function in `-lnanohttp':
#AC_CHECK_LIB([xml2], [xmlXPathFreeObject], [], [
#           echo "nanohttp library is required for Atlantis. Install package libxml2-dev"
#           exit -1])

# FIXME: Replace `main' with a function in `-lnetcdf':
AC_CHECK_LIB([netcdf], [ncopen], [],[
           echo "NetCDF library is required for Atlantis. Install package libnetcdf-dev"
           exit -1])

# FIXME: Replace `main' with a function in `-lproj':
AC_CHECK_LIB([proj], [pj_init], [],[
           echo "Proj4 library is required for Atlantis. Install package proj"
           exit -1])

# FIXME: Replace `main' with a function in `-lnanohttp':
AC_CHECK_LIB([xml2], [xmlXPathFreeObject], [], [
       echo "nanohttp library is required for Atlantis. Install package libxml2-dev"
       exit -1])

AC_ARG_ENABLE(brokerlink,
[  --enable-brokerlink   Turn on linkage with broker],
[
	case "${enableval}" in
  		yes) brokerlink=true ;;
  		no)  brokerlink=false ;;
  		*) AC_MSG_ERROR(bad value ${enableval} for --enable-brokerlink) ;
	esac
],
[brokerlink=false])
AM_CONDITIONAL(BROKER_LINK_ENABLED, test x$brokerlink = xtrue)


AC_ARG_ENABLE(link,
[  --enable-link   Turn on linkage without broker],
[
	case "${enableval}" in
  		yes) link=true ;;
  		no)  link=false ;;
  		*) AC_MSG_ERROR(bad value ${enableval} for --enable-link) ;
	esac



   #not sure why this is not working.
	# Check the csoap lib has been found:
	#AC_CHECK_LIB([csoap], [soap_ctx_new], [], [
	#           exit -1])
	AC_CHECK_LIB(csoap,printf,[LIBS="${LIBS} -lcsoap"])

	# FIXME: Replace `main' with a function in `-lnanohttp':
	AC_CHECK_LIB([nanohttp], [herror_code], [], [
	           echo "nanohttp library is required for Atlantis. Install package libnanohttp-dev"
	           exit -1])
	],[link=false])
AM_CONDITIONAL(LINK_ENABLED, test x$link = xtrue)


AC_ARG_ENABLE(clamlink,
[  --enable-clamlink   Turn on linkage with CLAM],
[case "${enableval}" in
  yes) clamlink=true ;;
  no)  clamlink=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-clamlink) ;
esac],[clamlink=false])
AM_CONDITIONAL(CLAM_LINK_ENABLED, test x$clamlink = xtrue)


AC_ARG_ENABLE(ss3link,
[  --enable-ss3link   Turn on linkage with SS3],
[case "${enableval}" in
  yes) ss3link=true ;;
  no)  ss3link=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-ss3link) ;
esac],[ss3link=false])
AM_CONDITIONAL(SS3_LINK_ENABLED, test x$ss3link = xtrue)

AC_ARG_ENABLE(rassesslink,
[  --enable-rassesslink   Turn on linkage with R based assessments],
[case "${enableval}" in
  yes) rassesslink=true ;;
  no)  rassesslink=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-rassesslink) ;
esac],[rassesslink=false])
AM_CONDITIONAL(RASSESS_LINK_ENABLED, test x$rassesslink = xtrue)

# Checks for header files.
m4_warn([obsolete],
[The preprocessor macro `STDC_HEADERS' is obsolete.
  Except in unusual embedded environments, you can safely include all
  ISO C90 headers unconditionally.])dnl
# Autoupdate added the next two lines to ensure that your configure
# script's behavior did not change.  They are probably safe to remove.
#AC_CHECK_INCLUDES_DEFAULT
AC_PROG_EGREP

AC_CHECK_HEADERS([float.h malloc.h netinet/in.h stddef.h stdlib.h string.h sys/socket.h unistd.h values.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.

AC_PROG_CC
AC_PROG_CXX
#AM_PROG_LIBTOOL
AM_SANITY_CHECK

#AC_C_CONST
#AC_HEADER_STDBOOL
#AC_C_INLINE
#AC_TYPE_SIZE_T
#AC_STRUCT_TM
#AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
m4_warn([obsolete],
[your code may safely assume C89 semantics that RETSIGTYPE is void.
Remove this warning and the `AC_CACHE_CHECK' when you adjust the code.])dnl
AC_CACHE_CHECK([return type of signal handlers],[ac_cv_type_signal],[AC_COMPILE_IFELSE(
[AC_LANG_PROGRAM([#include <sys/types.h>
#include <signal.h>
],
		 [return *(signal (0, 0)) (0) == 1;])],
		   [ac_cv_type_signal=int],
		   [ac_cv_type_signal=void])])
AC_DEFINE_UNQUOTED([RETSIGTYPE],[$ac_cv_type_signal],[Define as the return type of signal handlers
		    (`int' or `void').])

AC_FUNC_VPRINTF
AC_CHECK_FUNCS([floor localtime_r memset pow regcomp sqrt strcasecmp strchr strdup strerror strncasecmp strstr])

#fetching release tag from git
AC_MSG_CHECKING([for release version])
RELEASE_VERSION=$(git describe --tags 2>/dev/null || echo "unknown")
#RELEASE_VERSION=$(git describe --tags --abbrev=0 2>/dev/null || echo "unknown")
AC_MSG_RESULT([$RELEASE_VERSION])
AC_DEFINE_UNQUOTED([RELEASE_VERSION], ["$RELEASE_VERSION"], [Release version])
AC_SUBST([RELEASE_VERSION])
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([floor localtime_r memset pow regcomp sqrt strcasecmp strchr strdup strerror strncasecmp strstr])
#


AM_CONDITIONAL(GCC_GE_730, test `gcc -dumpversion | gawk '{print $1>=7?"1":"0"}'` = 1)

# identify the OS and the chip
# This macro sets several shell variables, most importantly $host_os,
# which contains a canonical name for the OS (e.g., linux-gnu, darwin, freebsd).
AC_CANONICAL_HOST
# Define a conditional flag for Linux
AM_CONDITIONAL([OS_LINUX], [case "$host_os" in *linux*) true ;; *) false ;; esac])

# Define a conditional flag for macOS (Darwin)
AM_CONDITIONAL([OS_MACOS], [case "$host_os" in *darwin*) true ;; *) false ;; esac])

# Define a conditional flag for FreeBSD
AM_CONDITIONAL([OS_FREEBSD], [case "$host_os" in *freebsd*) true ;; *) false ;; esac])



AC_CONFIG_FILES([Makefile
             ConvertAtlantis/Makefile
             atassess/Makefile
             atecology/Makefile
             ateconomic/Makefile
             atlantisUtil/Makefile
             atlantismain/Makefile
             atlantismain/atlantisMerged.pc
             atlink/Makefile
             atbrokerlink/Makefile
             atCLAMLink/Makefile
             atSS3Link/Makefile
             atmanage/Makefile
             atharvest/Makefile
             atimplementation/Makefile
             atphysics/Makefile
             sjwlib/Makefile])

AC_OUTPUT
