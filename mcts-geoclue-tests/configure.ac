AC_PREREQ(2.63)
AC_INIT(mcts-geoclue-tests, 1.0.0, qin.mu@intel.com)

AC_CONFIG_SRCDIR(src/test-provider/geoclue-test.c)
AM_INIT_AUTOMAKE(mcts-geoclue-tests, 1.0.0)

AC_PROG_CC
AC_ISC_POSIX

AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_GCC_TRADITIONAL
AM_PROG_CC_C_O

AC_ARG_ENABLE(system-bus,
	      [AC_HELP_STRING([--enable-system-bus],
			      [Use the system bus instead of session bus])],
	      enable_system_bus="$enableval",
	      enable_system_bus=no)


PKG_CHECK_MODULES(GEOCLUE, [
		   geoclue
])
AC_SUBST(GEOCLUE_LIBS)
AC_SUBST(GEOCLUE_CFLAGS)


AC_PATH_PROG(DBUS_BINDING_TOOL, dbus-binding-tool)
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)

DBUS_SERVICES_DIR="${datadir}/dbus-1/services"
AC_SUBST(DBUS_SERVICES_DIR)
AC_DEFINE_UNQUOTED(DBUS_SERVICES_DIR, "$DBUS_SERVICES_DIR", [Where services dir for D-Bus is])

CFLAGS="$CFLAGS -g -Wall -Werror -Wno-format"

PROVIDER_SUBDIRS="test"
AC_SUBST(PROVIDER_SUBDIRS)


TEST_BIN_DIR="/opt/${PACKAGE}-${VERSION}"
AC_SUBST(TEST_BIN_DIR)

XML_DATA_DIR="${datadir}${PACKAGE}"
AC_SUBST(XML_DATA_DIR)

AC_CONFIG_FILES([
Makefile
src/Makefile
src/geoclue-position/Makefile
src/geoclue-address/Makefile
src/geoclue-geocode/Makefile
src/geoclue-provider/Makefile
src/geoclue-accuracy/Makefile
src/geoclue-master/Makefile
src/test-provider/Makefile
utils/Makefile
])

AC_OUTPUT

echo ""
echo "mcts-geoclue-test  ${VERSION} has been configured as follows: "
echo "---------------------------------------------------"
echo "Source code location:   ${srcdir}"
echo "Compiler:               ${CC}"
echo "Providers:              ${PROVIDER_SUBDIRS}"
echo ""
