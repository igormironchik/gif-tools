qt_version=$(cat $PWD/script/qt.version)
resvg_version=$(cat $PWD/3rdparty/resvg.version)

echo "Installing Qt Installer Framework..."

aqt install-tool --outputdir ./Qt linux desktop tools_ifw qt.tools.ifw.47 || exit 1

echo "Copying binaries..."

rm -rf ./installer/packages/mironchik.igor.gif/data/bin

rm -rf ./installer/packages/mironchik.igor.gif/data/lib

rm -rf ./installer/packages/mironchik.igor.gif/data/plugins

rm -rf ./installer/packages/mironchik.igor.gif/data/libexec

rm -rf ./installer/packages/mironchik.igor.gif/data/translations

mkdir ./installer/packages/mironchik.igor.gif/data/bin || exit 1

mkdir ./installer/packages/mironchik.igor.gif/data/lib || exit 1

mkdir ./installer/packages/mironchik.igor.gif/data/plugins || exit 1

mkdir ./installer/packages/mironchik.igor.gif/data/libexec || exit 1

mkdir ./installer/packages/mironchik.igor.gif/data/translations || exit 1

cp ./build-gif-tools/bin/gif-editor ./installer/packages/mironchik.igor.gif/data/bin/gif-editor || exit 1

cp ./build-gif-tools/bin/gif-recorder ./installer/packages/mironchik.igor.gif/data/bin/gif-recorder || exit 1

cp -r ./Qt/$qt_version/gcc_64/lib ./installer/packages/mironchik.igor.gif/data || exit 1

cp -r ./Qt/$qt_version/gcc_64/plugins ./installer/packages/mironchik.igor.gif/data || exit 1

cp -r ./Qt/$qt_version/gcc_64/translations ./installer/packages/mironchik.igor.gif/data || exit 1

rm -rf ./installer/packages/mironchik.igor.gif/data/lib/cmake || exit 1

rm -rf ./installer/packages/mironchik.igor.gif/data/lib/metatypes || exit 1

rm -rf ./installer/packages/mironchik.igor.gif/data/lib/pkgconfig || exit 1

rm -rf ./installer/packages/mironchik.igor.gif/data/lib/*.prl || exit 1

echo "Creating installer..."

./Qt/Tools/QtInstallerFramework/4.7/bin/binarycreator -c ./installer/config/config.xml -p ./installer/packages GIF_Linux_x64.Installer || exit 1
