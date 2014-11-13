#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(IPVISUALIZER, 0.04.2, esk-ipvisualizer@esk.cs.usu.edu)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([client/constants.h])
AM_CONFIG_HEADER([config.h])

AM_MAINTAINER_MODE([enable])

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.
AC_CHECK_LIB([GL], [glEnable])
AC_CHECK_LIB([GLU], [gluOrtho2D])
AC_CHECK_LIB([SDL], [SDL_Init])
AC_CHECK_LIB([avcodec], [avcodec_open])
AC_CHECK_LIB([avformat], [avformat_alloc_context])
AC_CHECK_LIB([pcap], [pcap_open_live])
AC_CHECK_LIB([pthread], [pthread_create])

if test "$ac_cv_lib_SDL_main" == no
then
	AC_MSG_ERROR([Please install the SDL development libraries])
fi
if test "$ac_cv_lib_pcap_main" == no
then
	AC_MSG_ERROR([Please install the libpcap development libraries])
fi

AC_MSG_CHECKING([for libcurl])
if curl-config --libs > /dev/null
then
	AC_MSG_RESULT([yes])
else
	AC_MSG_RESULT([no])
	AC_MSG_ERROR([Please install the libcurl development libraries])
fi

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h netdb.h netinet/in.h stdlib.h string.h strings.h sys/socket.h sys/time.h unistd.h ffmpeg/avcodec.h ffmpeg/avformat.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_HEADER_TIME
AC_TYPE_UINT8_T
AC_C_VOLATILE

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([atexit gethostbyname gettimeofday memset socket strcasecmp strdup strerror strstr strtol])

AC_CONFIG_FILES([Makefile
                 client/Makefile
                 firewallsrv/Makefile
                 iptulog/Makefile
                 server/Makefile
                 shared/Makefile])
AC_OUTPUT
