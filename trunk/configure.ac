#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(IPVISUALIZER, 0.03, rian.shelley at usu.edu)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([client/constants.h])
AM_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lGL':
AC_CHECK_LIB([GL], [main])
# FIXME: Replace `main' with a function in `-lGLU':
AC_CHECK_LIB([GLU], [main])
# FIXME: Replace `main' with a function in `-lSDL':
AC_CHECK_LIB([SDL], [main])
# FIXME: Replace `main' with a function in `-lavcodec':
AC_CHECK_LIB([avcodec], [main])
# FIXME: Replace `main' with a function in `-lavformat':
AC_CHECK_LIB([avformat], [main])
# FIXME: Replace `main' with a function in `-lpcap':
AC_CHECK_LIB([pcap], [main])
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])

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
AC_CHECK_HEADERS([fcntl.h netdb.h netinet/in.h stdlib.h string.h strings.h sys/socket.h sys/time.h unistd.h])

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
