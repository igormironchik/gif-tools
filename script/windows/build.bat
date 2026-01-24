set /P qt_version=<%CD%\script\qt.version
set /P qt_arch=<%CD%\script\qt.arch.win

echo "Building gif-tools..."

mkdir build-gif-tools

cmake -DCMAKE_BUILD_TYPE=Release -S . -B build-gif-tools -DCMAKE_PREFIX_PATH=%CD%\Qt\%qt_version%\%qt_arch% -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build build-gif-tools --config Release

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)
