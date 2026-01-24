echo "Building gif-tools..."

mkdir build-gif-tools

qt_version=$(cat $PWD/script/qt.version)

build_type=$1

if [ -z "$1" ]; then
    build_type="Release"
fi

build_tests=$2

if [ -z "$2" ]; then
    build_tests="OFF"
fi

build_opts=""

if [ "$build_type" = "Debug" ]; then
    build_opts="-DENABLE_COVERAGE=ON"
fi

cmake $build_opts -DCMAKE_BUILD_TYPE=$build_type -S . -B build-gif-tools -DCMAKE_PREFIX_PATH=$PWD/Qt/$qt_version/gcc_64 || exit 1

cmake --build build-gif-tools --config Release || exit 1
