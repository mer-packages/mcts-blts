#! /bin/sh
CUR=`pwd`
#NAME=`cat debian/changelog | awk '{print $1; exit}'`
NAME=mwts-usb
BUILD=rpm/BUILD
SOURCE=rpm/SOURCES
VERSION=`cat debian/changelog | awk '{print $2; exit}' | sed -e "s/(//g" | sed -e "s/)//g"`

#Build source package
SRCPKGNAME=$NAME-$VERSION

#rm old temporary files, if script was stopped
# cannot remove all .spec is in ./rpm
#rm -Rf rpm/*
rm -Rf rpm/{BUILDROOT,BUILD,S{OURCE,PEC,RPM}S}
rm -Rf /var/tmp/$SRCPKGNAME

#Build source package
mkdir -p rpm/BUILD rpm/RPMS rpm/SOURCES
echo "Creating SRC package for $SRCPKGNAME..."
mkdir -p /var/tmp/$SRCPKGNAME
cp -r . /var/tmp/$SRCPKGNAME
cd /var/tmp/
tar czf $CUR/$SOURCE/$SRCPKGNAME.tar.gz $SRCPKGNAME
cd $CUR
rm -fr /var/tmp/$SRCPKGNAME

#Build RPM
echo "Building RPM packages..."
rpmbuild -v -bb --clean rpm/$NAME.spec --define "version $VERSION" --define "_topdir $CUR/rpm" --define "name $NAME"
echo "=========================================="
echo "$NAME RPM files can be located from rpm/RPMS"

#rm temporary files
rm -Rf rpm/{BUILDROOT,BUILD,S{OURCE,PEC,RPM}S}

