echo "Building gif-tools..."

qt_version=$(cat $PWD/script/qt.version)

mkdir build-gif-tools

cmake -DCMAKE_BUILD_TYPE=Release -S . -B build-gif-tools -DCMAKE_FIND_FRAMEWORK=LAST -DCMAKE_PREFIX_PATH="$PWD/Qt/$qt_version/macos;`brew --prefix`" || exit 1

cmake --build build-gif-tools --config Release || exit 1
