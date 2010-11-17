#! /bin/sh

CUR=`pwd`
NAME=`cat debian/changelog | awk '{print $1; exit}'`
TMP=rpm/tmp/
BUILD=rpm/BUILD
SOURCE=rpm/SOURCES
VERSION=`cat debian/changelog | awk '{print $2; exit}' | sed -e "s/(//g" | sed -e "s/)//g"`

#Build source package
SRCPKGNAME=$NAME-$VERSION

#rm old temporary files, if script was stopped
rm -Rf rpm/*
rm -Rf /var/tmp/$SRCPKGNAME

mkdir -p rpm/{BUILD,RPMS,S{OURCE,PEC,RPM}S}

echo "Creating SRC package for $SRCPKGNAME..."
echo "mkdir -p /var/tmp/$SRCPKGNAME"
mkdir -p /var/tmp/$SRCPKGNAME

echo "cp -r . /var/tmp/$SRCPKGNAME"
cp -r . /var/tmp/$SRCPKGNAME

cd /var/tmp/
echo "tar czf $CUR/$SOURCE/$SRCPKGNAME.tar.gz $SRCPKGNAME"
tar czf $CUR/$SOURCE/$SRCPKGNAME.tar.gz $SRCPKGNAME

cd $CUR
echo "rm -fr /var/tmp/$SRCPKGNAME"
rm -fr /var/tmp/$SRCPKGNAME

#Build RPM
echo "Building RPM packages..."
rpmbuild -v -bb --clean $NAME.spec --define "version $VERSION" --define "_topdir $CUR/rpm" --define "name $NAME"

#rm temporary files
rm -Rf rpm/{BUILDROOT,BUILD,S{OURCE,PEC,RPM}S}

echo "=========================================="
echo "$NAME RPM files can be located from rpm/RPMS"
