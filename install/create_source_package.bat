@echo off
::**************************************************************************
:: File:           create_source_package.bat
:: Version:        1.00
:: Name:           RJP Computing 
:: Date:           07/24/2006
:: Description:    creates a source directory so that the installer can
::                 include only the needed files.
::**************************************************************************

echo Cleaning...
echo.
if exist source del /Q source\*.*

echo Coping 'src' directory to 'source\src'
xcopy ..\src source\src /E /I /H /EXCLUDE:excludes

echo Coping 'libs' directory to 'source\libs'
xcopy ..\libs source\libs /E /I /H /EXCLUDE:excludes

echo Coping 'bin' directory to 'source\bin'
xcopy ..\bin source\bin /E /I /H /EXCLUDE:excludes

echo.
echo Finished source package...
