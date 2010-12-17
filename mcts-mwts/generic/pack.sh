#!/bin/sh

# Packaging script for a single BLTS test case.
#
# - Builds a tarball out of the sources
# - Builds a RPM package, if so requested
# - Builds a Debian package, if so requested
# - With a command line parameter, release dir can be specified, so
#   resulting package are copied to that destination

deb_packages=
rpm_packages=
source_package=
package_version=
exclude_pattern="*.tar.gz"
additional_dest=
no_rpm="yes"
no_debian="yes"
current=`pwd`

# Spam the usage
print_usage()
{
    echo "Script to build a source package release. By default"
    echo "only sources are packed, no building is done.\n"
    echo "Usage: $0 [-o <dir>][-r][-d][-h]"
    echo "\t-o\tCopy files (packages) to <dir>"
    echo "\t-r\tBuild RPM package, fails if there's no .spec"
    echo "\t-d\tBuild Debian package, fails if there's no debian/control"
    echo "\t-h|-?\tThis spam"
}

# Gets names of all rpm packages, assigns source package name
get_rpm_packages()
{
    local name=
    local subpackages=
    local is_full=

    name=`grep "Name:" *.spec | awk '{print $2}'`

    # Failure, maybe there's no RPM files
    if [ "x$name" = "x" ]; then
        echo "Error: Couldn't resolve package name, possibly no RPM packaging"
        return 1
    fi

    source_package=$name
    rpm_packages=$name

    subpackages=`grep "%package " *.spec | sed -e "s/%package//g"`

    for tok in $subpackages; do
        if [ "$tok" = "-n" ]; then
            is_full=1
            continue
        fi
        if [ $is_full = 1 ]; then
            is_full=0
            rpm_packages="$rpm_packages $tok"
        else
            rpm_packages="$rpm_packages $source_package-$tok"
        fi
    done

    package_version=`grep "Version:" *.spec | awk '{print $2}'`

    return 0
}

# Get all debian packages this package will generate
get_debian_packages()
{
    if [ ! -f debian/control ]; then
        echo "Error: Couldn't find debian/control file"
        return 1
    fi

    source_package=`grep "Source:" debian/control | awk '{print $2}'`
    deb_packages=`grep "Package:" debian/control | awk '{print $2}'`
    package_version=`grep 'blts' debian/changelog | awk '{print $2; exit}' | sed -e "s/[()]//g;s/-.*//g"`

    return 0
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
    # Parse RPM packages this package will create
    get_rpm_packages

    if [ $? = 1 ]; then
        echo "Failed to parse rpm packages"
        no_rpm="yes"
    fi
fi

# Parse name, version and release from debian files if there was no .spec
# file, or debian package are going to be built
if [ "x$no_debian" = "x" -o "x$source_package" = "x" ]; then
    get_debian_packages
fi

# Check if package name and version have been set
if [ "x$source_package" = "x" ]; then
    echo "Error: Couldn't determine source package name!"
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
rm -f $source_package-*.tar.gz

# Make a temporary sub dir
mkdir -vp /tmp/$source_package-$package_version

# This copy will throw a warning, "can't copy into itself"
cp -va * -t /tmp/$source_package-$package_version
cd /tmp

# Pack the stuff into tarball
tar --exclude="$exclude_pattern" -cvzf $current/$source_package-$package_version.tar.gz $source_package-$package_version

cd $current

if [ ! $? = 0 ]; then
    echo "Error: Failed to create tarball!"
    exit 1
fi

# To clean it up
rm -rf /tmp/$source_package-$package_version

# Do the additional copy if necessary
if [ ! "x$additional_dest" = "x" ]; then
    cp -v $source_package-$package_version.tar.gz $additional_dest

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
    cp -v $source_package-$package_version.tar.gz $dest

    if [ ! $? = 0 ]; then
        echo "Error: Failed to copy $source_package-$package_version.tar.gz to $dest!"
        exit 1
    fi

    # Then, build the stuff
    rpmbuild -ba $source_package.spec

    if [ ! $? = 0 ]; then
        echo "Error: Failed to rpmbuild $package_name!"
        exit 1
    fi

    # Copy the resulting packages into the destination give from command line
    if [ ! "x$additional_dest" = "x" ]; then
        for tok in $rpm_packages; do
            find $RPM_BUILD_DIR -name "$tok-$package_version*.rpm" | xargs -I '{}' cp -v '{}' $additional_dest
        done
    fi
fi

# Check from command line, is debian packaging to be done
if [ "x$no_debian" = "x" ]; then

    dpkg-buildpackage -D -rfakeroot

    if [ $? -gt 1 ]; then
        echo "Failed to build debian package for $source_package"
        exit 1
    fi

    # Copy the resulting package into the destination give from command line
    if [ ! "x$additional_dest" = "x" ]; then
        for tok in $deb_packages; do
            find .. -name "$tok*$package_version*.deb" | xargs -I '{}' cp -v '{}' $additional_dest
        done
        find .. -name "$source_package*$package_version*.dsc" | xargs -I '{}' cp -v '{}' $additional_dest
    fi

fi

exit 0
