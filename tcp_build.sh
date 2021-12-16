# BuildType=$1
# BuildOldOtaPath=$2
BUILDDIR=`pwd`
export BUILDDIR
echo $BUILDDIR
rm build -rf

BINDIR=$BUILDDIR/build/bin
echo $BINDIR
mkdir -p $BINDIR
export BINDIR

LIBDIR=$BUILDDIR/build/lib
echo $LIBDIR
mkdir -p $LIBDIR
export LIBDIR

make