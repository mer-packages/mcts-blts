#!/bin/sh
#
# Packaging script for a single BLTS test case.
#
# - Builds a tarball out of the sources, using an exclude pattern
# - Copies the tarball into rpm source dir, unless explicitly blocked by
#   command line switch

package_name=
package_version=
exclude_pattern="*.tar.gz"
additional_dest=
no_rpm=
no_build=
result=

USAGE="Usage: $0 [-n|-d <destination>|-h]\n"
USAGE="$USAGE\t-n\tDon't build RPM package\n"
USAGE="$USAGE\t-d\tAdditionally, copy the source package to <destination>\n"
USAGE="$USAGE\t-h|-?\tThis spam"

# Parse the one command line argument
while getopts nhd: f; do
    case $f in
        n) no_build="yes";;
        d) additional_dest=$OPTARG;;
        h | \?) echo $USAGE; exit 1;;
    esac
done
shift `expr $OPTIND - 1`

# Parse name and version from .spec file, if there is one
package_name=`grep "Name:" *.spec | awk '{print $2}'`
if [ "x$package_name" = "x" ]; then
    no_rpm="yes"
else
    package_version=`grep "Version:" *.spec | awk '{print $2}'`
fi

# Parse name, version and release from debian/changelog if there was no .spec
# file
if [ "x$no_rpm" = "xyes" ]; then
    package_name=`grep 'blts' debian/changelog | awk '{print $1; exit}'`
    package_version=`grep 'blts' debian/changelog | awk '{print $2; exit}' | sed -e "s/[()]//g;s/-.*//g"`
fi

# Check if package name and version have been set
if [ "x$package_name" = "x" ]; then
    echo "Error: Couldn't determine package name!\n"
    exit 1
fi

if [ "x$package_version" = "x" ]; then
    echo "Error: Couldn't determine package version!\n"
    exit 1
fi

# Try to clean up as much as possible before packing
if [ -f Makefile ]; then
    make clean
    make distclean
fi

# Clean (quietly) the old archives
rm -vf $package_name-*.tar.gz

# Make a temporary sub dir
mkdir -vp $package_name-$package_version

# This copy will throw a warning, "can't copy into itself"
cp -va * -t $package_name-$package_version

# Pack the stuff into tarball
tar --exclude="$exclude_pattern" -cvzf $package_name-$package_version.tar.gz $package_name-$package_version

if [ ! $? = 0 ]; then
    echo "Error: Failed to create tarball!\n"
    exit 1
fi

# To clean it up
rm -vrf $package_name-$package_version

# Do the additional copy if necessary
if [ ! "x$additional_dest" = "x" ]; then
    cp -v $package_name-$package_version.tar.gz $additional_dest

    if [ ! $? = 0 ]; then
        echo "Error: Failed to copy the tarball to $additional_dest!"
        exit 1
    fi
fi

# Check command line, is rpm packaging blocked
if [ "x$no_build" = "x" -a "x$no_rpm" = "x" ]; then

    # Determine the destination dir
    dest=""

    if [ "x$RPM_BUILD_DIR" = "x" ]; then
        dest="$HOME/rpmbuild/SOURCES"
    else
        dest="$RPM_BUILD_DIR/SOURCES"
    fi

    if [ ! -d $dest ]; then
        mkdir -vp $dest

        if [ ! $? = 0 ]; then
            echo "Error: Failed to create directory $dest\n"
            exit 1
        fi
    fi

    # Then, copy all packaged sources to build dir
    cp -v $package_name-$package_version.tar.gz $dest

    if [ ! $? = 0 ]; then
        echo "Error: Failed to copy $package_name-$package_version.tar.gz to $dest!\n"
        exit 1
    fi

    # Then, build the stuff
    rpmbuild -ba $package_name.spec

    if [ ! $? = 0 ]; then
        echo "Error: Failed to rpmbuild $package_name!\n"
        exit 1
    fi
fi

exit 0
