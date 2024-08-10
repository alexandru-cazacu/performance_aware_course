@echo off
setlocal enabledelayedexpansion

goto :main

:compile
	set      prjName=rdtsc_test
	set        files=%~dp0\rdtsc_test.cpp
	set     buildDir=%~dp0\build\Win-x64-%target%\
	set   exceptions=-EHa-
	set    subsystem=-subsystem:console

	echo Building %prjName% for %target%...

	set compilerFlags=^
		-Fe%prjName% ^
		-Fo%buildIntDir% ^
		-diagnostics:column ^
		-Oi -Gm- -GR- %exceptions% -FC -nologo -std:c++17 ^
		-fp:fast -fp:except- ^
		-W4 -WX -wd4189 -wd4505 ^
		-D_CRT_SECURE_NO_WARNINGS

	set   debugCompilerFlags=-MDd -Zo -Zi
	set releaseCompilerFlags=-MD -GL -O2

	set linkerFlags=-WX -opt:ref -incremental:no %subsystem%
		
	set  sharedLibs=kernel32.lib shell32.lib user32.lib
	set  debugLibs=
	set  releaseLibs=

	:: Create Build directory
	if not exist %buildDir% mkdir %buildDir%

	:: call ..\tools\ctime\ctime -begin %prjName%_Win-x64-%target%.ctime
	pushd %buildDir%
		del %prjName%.pdb > NUL 2> NUL
		del %prjName%.* > NUL 2> NUL

		if %target%==Debug (
			:: Compile
			cl %compilerFlags% %debugCompilerFlags% %files% /link %linkerFlags% %sharedLibs% %debugLibs%
		)

		if %target%==Release (
			:: Compile
			cl %compilerFlags% %releaseCompilerFlags% %files% /link %linkerFlags% %sharedLibs% %releaseLibs%
		)

	popd
	:: call ..\tools\ctime\ctime -end %prjName%_Win-x64-%target%.ctime !ERRORLEVEL!

	goto :exit

:print_help 
	echo Usage: %~n0%~x0 [options]
	echo.
	echo   Compiles the project.
	echo   If no argument is passed it compiles for Debug.
	echo   If --release is passed it compiles for Release.
	echo   If --tests is passed it compiles Unit Tests.
	echo.
	echo Options:
	echo   -h, --help           Display this message and exit.
	echo   --release            Release build.
	echo   --debug              Debug build.

	goto :exit

:main
	if "%1"=="--help" (
		call :print_help
	) else if "%1"=="-h" (
		call :print_help
	) else if "%1"=="--release" (
		set target=Release
		call :compile
	) else if "%1"=="--debug" (
		set target=Debug
		call :compile
	) else if "%1"=="" (
		set target=Debug
		call :compile
	) else (
		echo Error. Unknow option "%1".
		exit /B -1
	)

:exit
	exit /B !ERRORLEVEL!

:: ╔════════════════╗
:: ║ Compiler Flags ║
:: ╚════════════════╝

:: Oi            Generates intrinsic functions.
:: O2            Creates fast code.
:: GR-           Disables run-time type information (RTTI).
:: GL            Enables whole program optimization.
:: Gm-           Disables minimal rebuild.
:: EHa-          Disables exception handling.
:: Fd:<file>     Renames program database file.
:: Fe:<file.exe> Renames the executable file.
:: Fo:<file>     Creates an object file.
:: Fm:<file>     Creates a mapfile.
:: D <name>      Defines constants and macros.
:: I <dir>       Searches a directory for include files.
:: Zi            Generates complete debugging information.
:: Zo            Generates enhanced debugging information for optimized code.
:: LD            Creates a dynamic-link library.
:: MD            Compiles to create a multithreaded DLL, by using MSVCRT.lib.
:: FC            Display full path of source code files passed to cl.exe in diagnostic text.
:: nologo        Suppresses display of sign-on banner
:: WX            Treats all warnings as errors.
:: W4            Sets output warning level.
:: wd<nnn>       Disables the specified warning.

:: ╔═══════════════════╗
:: ║ Compiler Warnings ║
:: ╚═══════════════════╝

:: 4201 nonstandard extension used : nameless struct/union
:: 4100 'identifier' : unreferenced formal parameter
:: 4189 'identifier' : local variable is initialized but not referenced (will be optimized away by the compiler).

:: ╔══════════════╗
:: ║ Linker Flags ║
:: ╚══════════════╝

:: incremental:no Controls incremental linking (disabled).
:: opt:ref        Controls LINK optimizations (remove non referenced functions and data).
:: subsystem      Tells the operating system how to run the .exe file.
:: LIBPATH:<dir>  Specifies a path to search before the environmental library path.
