# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
# Authors:
#       Jenny Cao <jenny.q.cao@intel.com>
#       Shao-Feng Tang <shaofeng.tang@intel.com>
#
# Date Created: 09-16-2010
#       Modified from sample Makefile
AC_PREREQ(2.63)
AC_INIT(mcts-lsw-tests, 1.0.1, shaofeng.tang@intel.com)
AM_INIT_AUTOMAKE(mcts-lsw-tests, 1.0.1)

# Checks for programs.
AC_PROG_CC
AC_ISC_POSIX

AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_GCC_TRADITIONAL
AM_PROG_CC_C_O

PKG_CHECK_MODULES(LSW, [
       libsocialweb-keyfob
       libsocialweb-keystore
       libsocialweb-module
       libsocialweb-client
       gnome-keyring-1
])
AC_SUBST(LSW_LIBS)
AC_SUBST(LSW_CFLAGS)

AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)

CFLAGS="$CFLAGS -g -Wall -Werror -Wno-format"

AC_CONFIG_FILES([
Makefile  
src/Makefile 
src/core/Makefile 
src/lastfm/Makefile 
src/key_mgr/Makefile 
src/dbus_api/Makefile 
src/dbus_api/lswdbus/Makefile 
src/dbus_api/model/Makefile 
utils/Makefile src/qt/Makefile
])
AC_OUTPUT

