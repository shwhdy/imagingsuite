CPCMD="ln -s "
#CPCMD="cp "

target="Debug"
platform='unknown'
unamestr=`uname`
if [[ "$unamestr" == 'Linux' ]]; then
   DEST="imageviewer-build-desktop-Qt_4_8_1_in_PATH__System__Release/"
   REPOSPATH="/home/kaestner/repos"
   EXT="so.1.0.0"
   EXT1="so.1.0"
   EXT2="so.1"
elif [[ "$unamestr" == 'Darwin' ]]; then
   DEST="../build-imageviewer-Qt5-$target/imageviewer.app/Contents/MacOS"
   REPOSPATH="/Users/kaestner/repos"
   EXT="1.0.0.dylib"
   EXT1="1.0.dylib"
   EXT2="1.dylib"
fi

pushd .
cd $DEST
rm -f *.$EXT


`$CPCMD $REPOSPATH/modules/trunk/ModuleConfig/build-ModuleConfig-Qt5-$target/libModuleConfig.$EXT .`
`$CPCMD $REPOSPATH/modules/trunk/build-ReaderConfig-Qt5-$target/libReaderConfig.$EXT .`
`$CPCMD $REPOSPATH/modules/trunk/build-ReaderGUI-Qt5-$target/libReaderGUI.$EXT .`
`$CPCMD $REPOSPATH/gui/trunk/qt/build-QtAddons-Qt5-$target/libQtAddons.$EXT .`
`$CPCMD $REPOSPATH/kipl/trunk/kipl/build-kipl-Qt5-$target/libkipl.$EXT .`
`$CPCMD $REPOSPATH/qni/trunk/src/ImagingAlgorithms/build-ImagingAlgorithms-Qt5-$target/libImagingAlgorithms.$EXT .`

rm -f *.$EXT1
rm -f *.$EXT2

for f in `ls *.$EXT`; do
	bn=`basename $f .$EXT`
	echo $bn
	ln -s $f $bn.$EXT1
    ln -s $bn.$EXT1 $bn.$EXT2
done

popd