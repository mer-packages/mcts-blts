#!/bin/sh

# Packaging script for a single BLTS test case.
#
# - Builds a tarball out of the sources
# - Builds a RPM package, if so requested
# - Builds a Debian package, if so requested
# - With a command line parameter, release dir can be specified, so
#   resulting package are copied to that destination

package_name=
package_version=
exclude_pattern="*.tar.gz"
additional_dest=
no_rpm="yes"
no_debian="yes"
current=`pwd`

# Spam the usage
print_usage()
{
    echo "Usage: $0 [-o <dir>|-r|-d|-h]"
    echo "\nScript to build a source package release. By default"
    echo "only sources are packed, no building is done.\n"
    echo "\t-o\tCopy files (packages) to <dir>"
    echo "\t-r\tBuild RPM package, fails if there's no .spec"
    echo "\t-d\tBuild Debian package, fails if there's no debian/control"
    echo "\t-h|-?\tThis spam"
}

# Parse command line arguments
while getopts no:rdh f; do
    case $f in
        o) additional_dest=$OPTARG;;
        r) no_rpm=;;
        d) no_debian=;;
        h | \?) print_usage; exit 1;;
    esac
done
shift `expr $OPTIND - 1`

if [ "x$no_rpm" = "x" ]; then
    # Parse name and version from .spec file, if there is one
    package_name=`grep "Name:" *.spec | awk '{print $2}'`
    if [ ! "x$package_name" = "x" ]; then
        package_version=`grep "Version:" *.spec | awk '{print $2}'`
    else
        # Can't build RPM
        no_rpm="yes"
    fi
fi

# Parse name, version and release from debian/changelog if there was no .spec
# file
if [ "x$no_debian" = "x" -o "x$package_name" = "x" ]; then
    package_name=`grep 'blts' debian/changelog | awk '{print $1; exit}'`
    package_version=`grep 'blts' debian/changelog | awk '{print $2; exit}' | sed -e "s/[()]//g;s/-.*//g"`
fi

# Check if package name and version have been set
if [ "x$package_name" = "x" ]; then
    echo "Error: Couldn't determine package name!"
    exit 1
fi

if [ "x$package_version" = "x" ]; then
    echo "Error: Couldn't determine package version!"
    exit 1
fi

# Try to clean up as much as possible before packing
if [ -f Makefile ]; then
    make -s clean

    if [ -x debian/rules ]; then
        fakeroot debian/rules clean
    fi
fi

# Clean (quietly) the old archives
rm -f $package_name-*.tar.gz

# Make a temporary sub dir
mkdir -vp /tmp/$package_name-$package_version

# This copy will throw a warning, "can't copy into itself"
cp -va * -t /tmp/$package_name-$package_version
cd /tmp

# Pack the stuff into tarball
tar --exclude="$exclude_pattern" -cvzf $current/$package_name-$package_version.tar.gz $package_name-$package_version

cd $current

if [ ! $? = 0 ]; then
    echo "Error: Failed to create tarball!"
    exit 1
fi

# To clean it up
rm -rf /tmp/$package_name-$package_version

# Do the additional copy if necessary
if [ ! "x$additional_dest" = "x" ]; then
    cp -v $package_name-$package_version.tar.gz $additional_dest

    if [ ! $? = 0 ]; then
        echo "Error: Failed to copy the tarball to $additional_dest!"
        exit 1
    fi
fi

# Check command line, is rpm packaging blocked
if [ "x$no_rpm" = "x" ]; then

    # Determine the destination dir
    dest=""

    if [ "x$RPM_BUILD_DIR" = "x" ]; then
        RPM_BUILD_DIR="$HOME/rpmbuild"
        dest="$HOME/rpmbuild/SOURCES"
    else
        dest="$RPM_BUILD_DIR/SOURCES"
    fi

    if [ ! -d $dest ]; then
        mkdir -vp $dest

        if [ ! $? = 0 ]; then
            echo "Error: Failed to create directory $dest"
            exit 1
        fi
    fi

    # Then, copy all packaged sources to build dir
    cp -v $package_name-$package_version.tar.gz $dest

    if [ ! $? = 0 ]; then
        echo "Error: Failed to copy $package_name-$package_version.tar.gz to $dest!"
        exit 1
    fi

    # Then, build the stuff
    rpmbuild -ba $package_name.spec

    if [ ! $? = 0 ]; then
        echo "Error: Failed to rpmbuild $package_name!"
        exit 1
    fi

    # Copy the resulting package into the destination give from command line
    if [ ! "x$additional_dest" = "x" ]; then
        find $RPM_BUILD_DIR -name "$package_name-$package_version*.rpm" | xargs -I '{}' cp -v '{}' $additional_dest
    fi
fi

# Check from command line, is debian packaging to be done
if [ "x$no_debian" = "x" ]; then

    dpkg-buildpackage -D -rfakeroot

    if [ ?$ = 0 ]; then
        echo "Failed to build debian package for $package name"
        exit 1
    fi

    # Copy the resulting package into the destination give from command line
    if [ ! "x$additional_dest" = "x" ]; then
        find .. -name "$package_name*$package_version*.deb" | xargs -I '{}' cp -v '{}' $additional_dest
        find .. -name "$package_name*$package_version*.dsc" | xargs -I '{}' cp -v '{}' $additional_dest
    fi

fi

exit 0
