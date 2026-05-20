set /P qt_version=<%CD%\script\qt.version
set /P qt_arch=<%CD%\script\qt.arch.win

set "current_dir=%CD%"

set "cwd=%current_dir:\=/%"

echo "Building gif-tools..."

mkdir build-gif-tools

cmake -DCMAKE_BUILD_TYPE=Release -S . -B "%cwd%/../builds/build-gif-tools" -DCMAKE_PREFIX_PATH="%cwd%/../Qt/%qt_version%/%qt_arch%;%cwd%/../KDE" -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build "%cwd%/../builds/build-gif-tools" --config Release

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)
