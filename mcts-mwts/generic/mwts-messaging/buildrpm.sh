#! /bin/sh

CUR=`pwd`
NAME=`pwd | sed "s/.*\\///"`
TMP=rpm/tmp/
BUILD=rpm/BUILD
SOURCE=rpm/SOURCES
VERSION=`cat debian/changelog | awk '{print $2; exit}' | sed -e "s/(//g" | sed -e "s/)//g"`

#Build source package
SRCPKGNAME=$NAME\-$VERSION
mkdir -p rpm/{BUILD,RPMS,S{OURCE,PEC,RPM}S}
echo "Creating SRC package for $SRCPKGNAME..."
mkdir -p /var/tmp/$SRCPKGNAME
cp -r . /var/tmp/$SRCPKGNAME
cd /var/tmp/
tar czf $CUR/$SOURCE/$SRCPKGNAME.tar.gz $SRCPKGNAME
cd $CUR
rm -fr /var/tmp/$SRCPKGNAME

#Build RPM
echo "Building RPM packages..."
rpmbuild -v -bb --clean `ls rpm/*.spec` --define "ver $VERSION" --define "_topdir $CUR/rpm"

echo "=========================================="
echo "$NAME RPM files can be located from rpm/RPMS"
