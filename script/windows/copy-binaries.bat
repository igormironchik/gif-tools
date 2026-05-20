set /P qt_version=<%CD%\script\qt.version
set /P qt_arch=<%CD%\script\qt.arch.win

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

copy /Y ..\Qt\%qt_version%\%qt_arch%\bin installer\packages\mironchik.igor.gif\data\bin

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

copy /Y ..\builds\build-gif-tools\bin installer\packages\mironchik.igor.gif\data\bin

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

xcopy ..\Qt\%qt_version%\%qt_arch%\plugins installer\packages\mironchik.igor.gif\data\plugins /S /Y

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

xcopy ..\Qt\%qt_version%\%qt_arch%\translations installer\packages\mironchik.igor.gif\data\translations /S /Y

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

copy /Y 3rdparty\Windows\VC_Redist\VC_redist.x64.exe installer\packages\mironchik.igor.gif\data\bin\VC_redist.x64.exe

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir installer\packages\mironchik.igor.gif\data\bin\data

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

xcopy %CD%\..\KDE\bin\data installer\packages\mironchik.igor.gif\data\bin\data /S /Y

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir installer\packages\mironchik.igor.gif\data\plugins\kf6

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir installer\packages\mironchik.igor.gif\data\plugins\kf6\kio

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir installer\packages\mironchik.igor.gif\data\plugins\kf6\kio_dnd

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir installer\packages\mironchik.igor.gif\data\plugins\kf6\urifilters

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir installer\packages\mironchik.igor.gif\data\plugins\kiconthemes6

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir installer\packages\mironchik.igor.gif\data\plugins\kiconthemes6\iconengines

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir installer\packages\mironchik.igor.gif\data\plugins\styles

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

xcopy %CD%\..\KDE\lib\plugins\kf6 installer\packages\mironchik.igor.gif\data\plugins\kf6 /S /Y

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

copy /Y %CD%\..\KDE\bin\kioworker.exe installer\packages\mironchik.igor.gif\data\bin\kioworker.exe

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

xcopy %CD%\..\KDE\lib\plugins\kiconthemes6 installer\packages\mironchik.igor.gif\data\plugins\kiconthemes6 /S /Y

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

xcopy %CD%\..\KDE\lib\plugins\styles installer\packages\mironchik.igor.gif\data\plugins\styles /S /Y

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

xcopy %CD%\..\KDE\bin\*.dll installer\packages\mironchik.igor.gif\data\bin /S /Y

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

xcopy ..\Qt\%qt_version%\%qt_arch%\plugins installer\packages\mironchik.igor.gif\data\plugins /S /Y

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

xcopy ..\Qt\%qt_version%\%qt_arch%\resources installer\packages\mironchik.igor.gif\data\resources /S /Y

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

xcopy ..\Qt\%qt_version%\%qt_arch%\translations installer\packages\mironchik.igor.gif\data\translations /S /Y

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)