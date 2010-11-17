#! /bin/sh

RPM_ARCH="i586"

CUR=`pwd`
NAME=`cat debian/changelog | awk '{print $1; exit}'`
TMP=rpm/tmp/
BUILD=rpm/BUILD
SOURCE=rpm/SOURCES
VERSION=`cat debian/changelog | awk '{print $2; exit}' | sed -e "s/(//g" | sed -e "s/)//g"`

#Build source package
SRCPKGNAME=$NAME-$VERSION
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
#rpmbuild -v --target i586 -bb --clean $NAME.spec --define "version $VERSION" --define "_topdir $CUR/rpm" --define "name $NAME"
rpmbuild -v -bb --clean $NAME.spec --define "version $VERSION" --define "_topdir $CUR/rpm" --define "name $NAME"

echo "=========================================="
echo "$NAME RPM files can be located from rpm/RPMS"
