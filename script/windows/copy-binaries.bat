set /P qt_version=<%CD%\script\qt.version
set /P qt_arch=<%CD%\script\qt.arch.win
set /P resvg_version=<%CD%\3rdparty\resvg.version

echo "Copying binaries..."

rmdir /S /Q installer\packages\mironchik.igor.gif\data\bin

rmdir /S /Q installer\packages\mironchik.igor.gif\data\plugins

rmdir /S /Q installer\packages\mironchik.igor.gif\data\libexec

rmdir /S /Q installer\packages\mironchik.igor.gif\data\translations

mkdir installer\packages\mironchik.igor.gif\data\bin

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir installer\packages\mironchik.igor.gif\data\plugins

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir installer\packages\mironchik.igor.gif\data\translations

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

copy /Y Qt\%qt_version%\%qt_arch%\bin installer\packages\mironchik.igor.gif\data\bin

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

del /Q installer\packages\mironchik.igor.gif\data\bin\*.exe

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

del /Q installer\packages\mironchik.igor.gif\data\bin\*.bat

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

del /Q installer\packages\mironchik.igor.gif\data\bin\*.cmake

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

del /Q installer\packages\mironchik.igor.gif\data\bin\*.pl

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

del /Q installer\packages\mironchik.igor.gif\data\bin\*.py

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

del /Q installer\packages\mironchik.igor.gif\data\bin\*.sh

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

del /Q installer\packages\mironchik.igor.gif\data\bin\Qt6*d.dll

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

copy /Y build-gif-tools\bin installer\packages\mironchik.igor.gif\data\bin

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

xcopy Qt\%qt_version%\%qt_arch%\plugins installer\packages\mironchik.igor.gif\data\plugins /S /Y

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

xcopy Qt\%qt_version%\%qt_arch%\translations installer\packages\mironchik.igor.gif\data\translations /S /Y

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

copy /Y 3rdparty\Windows\VC_Redist\VC_redist.x64.exe installer\packages\mironchik.igor.gif\data\bin\VC_redist.x64.exe

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)
