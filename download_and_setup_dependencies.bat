@echo off
setlocal enabledelayedexpansion

REM Shape-Force - Dependency Download Script (Windows)
REM This script downloads and extracts all required third-party libraries
REM and automatically installs missing dependencies

REM Colors for output (Windows console color codes)
set "RED=[91m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "BLUE=[94m"
set "NC=[0m"

REM Function-like labels for colored output
goto :main

:print_status
echo %BLUE%[INFO]%NC% %~1
goto :eof

:print_success
echo %GREEN%[SUCCESS]%NC% %~1
goto :eof

:print_warning
echo %YELLOW%[WARNING]%NC% %~1
goto :eof

:print_error
echo %RED%[ERROR]%NC% %~1
goto :eof

:winget_install_or_skip
REM Usage: call :winget_install_or_skip "winget install --id ..."

setlocal
set "cmd=%~1"
set "logfile=%TEMP%\winget_install_log.txt"

call :print_status "Running: %cmd%"
%cmd% >"%logfile%" 2>&1

findstr /C:"Found an existing package already installed" "%logfile%" >nul
if %errorlevel% equ 0 (
    call :print_warning "Package already installed. Skipping upgrade."
    endlocal & exit /b 0
)

findstr /C:"Successfully installed" "%logfile%" >nul
if %errorlevel% equ 0 (
    call :print_success "Package installed successfully"
    endlocal & exit /b 0
)

type "%logfile%"
call :print_error "Winget install failed."
endlocal & exit /b 1

:check_and_install_dependencies
call :print_status "Checking and installing missing dependencies..."

REM Check if winget is available (Windows Package Manager)
winget --version >nul 2>&1
if !errorlevel! neq 0 (
    call :print_warning "winget not available. Please install dependencies manually:"
    call :print_warning "- Git: https://git-scm.com/download/win"
    call :print_warning "- Python: https://www.python.org/downloads/"
    call :print_warning "- CMake: https://cmake.org/download/"
    call :print_warning "- Visual Studio Build Tools or Visual Studio"
    echo.
    set /p "continue=Continue anyway? (y/N): "
    if /i not "!continue!"=="y" (
        call :print_error "Installation cancelled"
        exit /b 1
    )
    goto :skip_auto_install
)

REM Check and install Git
git --version >nul 2>&1
if !errorlevel! neq 0 (
    call :print_status "Installing Git..."
    winget install --id Git.Git -e --silent --accept-package-agreements --accept-source-agreements
    if !errorlevel! neq 0 (
        call :print_error "Failed to install Git via winget"
        exit /b 1
    )
    call :print_success "Git installed successfully"
    call :refresh_path
    
    REM Verify installation worked
    git --version >nul 2>&1
    if !errorlevel! neq 0 (
        call :force_refresh_and_retry
        git --version >nul 2>&1
        if !errorlevel! neq 0 (
            call :print_error "Git installation failed - command not found after installation"
            call :print_warning "You may need to restart your command prompt or reboot"
            exit /b 1
        )
    )
) else (
    call :print_success "Git is already installed"
)

REM Check and install Python
python --version >nul 2>&1
set "python_installed=!errorlevel!"
python3 --version >nul 2>&1
set "python3_installed=!errorlevel!"

if !python_installed! neq 0 if !python3_installed! neq 0 (
    call :print_status "Installing Python..."
    winget install --id Python.Python.3.12 -e --silent --accept-package-agreements --accept-source-agreements
    if !errorlevel! neq 0 (
        call :print_error "Failed to install Python via winget"
        exit /b 1
    )
    call :print_success "Python installed successfully"
    call :refresh_path
    
    REM Verify installation worked
    python --version >nul 2>&1
    set "python_check=!errorlevel!"
    python3 --version >nul 2>&1
    set "python3_check=!errorlevel!"
    
    if !python_check! neq 0 if !python3_check! neq 0 (
        call :force_refresh_and_retry
        python --version >nul 2>&1
        set "python_check=!errorlevel!"
        python3 --version >nul 2>&1
        set "python3_check=!errorlevel!"
        
        if !python_check! neq 0 if !python3_check! neq 0 (
            call :print_error "Python installation failed - command not found after installation"
            call :print_warning "You may need to restart your command prompt or reboot"
            exit /b 1
        )
    )
) else (
    call :print_success "Python is already installed"
)

REM Check and install CMake
cmake --version >nul 2>&1
if !errorlevel! neq 0 (
    call :print_status "Installing CMake..."
    winget install --id Kitware.CMake -e --silent --accept-package-agreements --accept-source-agreements
    if !errorlevel! neq 0 (
        call :print_error "Failed to install CMake via winget"
        exit /b 1
    )
    call :print_success "CMake installed successfully"
    call :refresh_path
    
    REM Verify installation worked
    cmake --version >nul 2>&1
    if !errorlevel! neq 0 (
        call :force_refresh_and_retry
        cmake --version >nul 2>&1
        if !errorlevel! neq 0 (
            call :print_error "CMake installation failed - command not found after installation"
            call :print_warning "You may need to restart your command prompt or reboot"
            exit /b 1
        )
    )
) else (
    call :print_success "CMake is already installed"
)

REM Check for Visual Studio Build Tools or Visual Studio
call :print_status "Checking for Visual Studio Build Tools..."

REM Check if MSVC compiler is already available in PATH
where cl >nul 2>&1
if !errorlevel! equ 0 (
    call :print_success "MSVC compiler detected (cl.exe in PATH)"
    goto :verify_cpp_setup
)

REM Check if Developer Command Prompt environment is already active
if defined VCINSTALLDIR (
    call :print_success "MSVC environment already set (VCINSTALLDIR detected)"
    goto :verify_cpp_setup
)

REM Use vswhere to detect installed VS instances
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        set "VSINSTALL=%%i"
    )
)

if defined VSINSTALL (
    call :print_success "Visual Studio with Build Tools already installed at: !VSINSTALL!"
    goto :setup_vs_environment
)

REM Install Visual Studio Build Tools with C++ workload
call :print_status "Installing Visual Studio Build Tools with C++ components..."
call :print_status "This may take several minutes..."

REM Try the more complete installation with specific C++ workload
call :print_status "Attempting installation with C++ workload..."
winget install --id Microsoft.VisualStudio.2022.BuildTools --override "--wait --add Microsoft.VisualStudio.Workload.VCTools --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 --add Microsoft.VisualStudio.Component.Windows11SDK.22621 --add Microsoft.VisualStudio.Component.VC.CMake.Project --includeRecommended" --silent --accept-package-agreements --accept-source-agreements
if !errorlevel! neq 0 (
    call :print_warning "Full workload installation failed, trying basic installation..."
    call :winget_install_or_skip "winget install --id Microsoft.VisualStudio.2022.BuildTools -e --silent --accept-package-agreements --accept-source-agreements"
    if !errorlevel! neq 0 (
        call :print_error "Failed to install Visual Studio Build Tools"
        exit /b 1
    )
) else (
    call :print_success "Visual Studio Build Tools with C++ workload installed successfully"
)

call :print_success "Visual Studio Build Tools installation completed"

REM Wait a moment for installation to settle
timeout /t 5 /nobreak >nul 2>&1

:setup_vs_environment
REM Find and set up Visual Studio environment
call :print_status "Setting up Visual Studio environment..."

REM Re-detect VS installation after potential install
if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        set "VSINSTALL=%%i"
    )
)

if defined VSINSTALL (
    REM Look for vcvarsall.bat
    set "VCVARSALL=!VSINSTALL!\VC\Auxiliary\Build\vcvarsall.bat"
    if exist "!VCVARSALL!" (
        call :print_success "Found vcvarsall.bat at: !VCVARSALL!"
        call :print_status "Setting up x64 build environment..."
        call "!VCVARSALL!" x64 >nul 2>&1
        if !errorlevel! equ 0 (
            call :print_success "Visual Studio environment configured"
        ) else (
            call :print_warning "Failed to configure VS environment automatically"
        )
    )
) else (
    call :print_warning "Could not locate Visual Studio installation path"
)

:verify_cpp_setup
REM Verify C++ compilation capabilities
call :print_status "Verifying C++ compilation setup..."

REM Check if cl.exe is now available
where cl >nul 2>&1
if !errorlevel! equ 0 (
    call :print_success "MSVC compiler (cl.exe) is available"
    
    REM Get compiler version
    for /f "tokens=*" %%i in ('cl 2^>^&1 ^| findstr /C:"Microsoft (R) C/C++ Optimizing Compiler"') do (
        call :print_status "Compiler version: %%i"
    )
    
    REM Test basic compilation
    call :test_basic_compilation
) else (
    call :print_warning "MSVC compiler not found in PATH"
    call :print_status "You may need to:"
    echo   1. Restart your command prompt
    echo   2. Or run this script from a "Developer Command Prompt"
    echo   3. Or manually run: "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
)

REM Check for Windows SDK
if defined WindowsSdkDir (
    call :print_success "Windows SDK detected: !WindowsSdkDir!"
) else (
    call :print_warning "Windows SDK not detected in environment"
)

goto :skip_vs_install

:test_basic_compilation
call :print_status "Testing basic C++ compilation..."

REM Create a simple test file
set "TEST_FILE=%TEMP%\test_cpp_compile.cpp"
echo #include ^<iostream^> > "%TEST_FILE%"
echo int main() { std::cout ^<^< "Hello, C++!" ^<^< std::endl; return 0; } >> "%TEST_FILE%"

REM Try to compile it
set "TEST_EXE=%TEMP%\test_cpp_compile.exe"
cl "%TEST_FILE%" /Fe:"%TEST_EXE%" /nologo >nul 2>&1

if !errorlevel! equ 0 (
    call :print_success "C++ compilation test passed"
    
    REM Run the test executable
    "%TEST_EXE%" >nul 2>&1
    if !errorlevel! equ 0 (
        call :print_success "C++ execution test passed"
    ) else (
        call :print_warning "C++ compilation succeeded but execution failed"
    )
    
    REM Clean up
    if exist "%TEST_EXE%" del "%TEST_EXE%" >nul 2>&1
) else (
    call :print_warning "C++ compilation test failed"
    call :print_status "This might be due to missing Windows SDK or incomplete installation"
)

REM Clean up test file
if exist "%TEST_FILE%" del "%TEST_FILE%" >nul 2>&1

goto :eof

:skip_vs_install

:skip_auto_install
call :print_success "Dependency check completed"
goto :eof

:refresh_path
REM Refresh PATH environment variable for current session
call :print_status "Refreshing environment variables..."

REM Get system PATH
for /f "tokens=2*" %%i in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v PATH 2^>nul') do set "sys_path=%%j"

REM Get user PATH  
for /f "tokens=2*" %%i in ('reg query "HKCU\Environment" /v PATH 2^>nul') do set "user_path=%%j"

REM Combine and set new PATH
if defined user_path (
    set "PATH=%sys_path%;%user_path%"
) else (
    set "PATH=%sys_path%"
)

REM Add common installation paths that might not be in registry yet
set "PATH=%PATH%;%ProgramFiles%\Git\bin"
set "PATH=%PATH%;%ProgramFiles%\Git\cmd"
set "PATH=%PATH%;%LocalAppData%\Programs\Python\Python312"
set "PATH=%PATH%;%LocalAppData%\Programs\Python\Python312\Scripts"
set "PATH=%PATH%;%ProgramFiles%\Python312"
set "PATH=%PATH%;%ProgramFiles%\Python312\Scripts"
set "PATH=%PATH%;%ProgramFiles%\CMake\bin"
set "PATH=%PATH%;%ProgramFiles(x86)%\CMake\bin"

REM Remove duplicate semicolons
set "PATH=%PATH:;;=;%"

call :print_success "Environment variables refreshed"
goto :eof

:force_refresh_and_retry
REM Force refresh PATH and retry command
call :print_warning "Tool not found after installation, forcing environment refresh..."
call :refresh_path

REM Wait a moment for system to settle
timeout /t 2 /nobreak >nul 2>&1

goto :eof

:detect_platform
set "PLAT=windows"
call :print_status "Detected platform: %PLAT%"
goto :eof

:create_directories
call :print_status "Creating directory structure: thirdparties\%PLAT%\"
# Delete all files and subfolders inside thirdparties
del /f /s /q thirdparties\*.*
for /d %%i in (thirdparties\*) do rmdir /s /q "%%i"

if not exist "thirdparties\%PLAT%\imgui" mkdir "thirdparties\%PLAT%\imgui"
if not exist "thirdparties\%PLAT%\glfw" mkdir "thirdparties\%PLAT%\glfw"
if not exist "thirdparties\%PLAT%\gl3w" mkdir "thirdparties\%PLAT%\gl3w"
if not exist "thirdparties\%PLAT%\spd_log" mkdir "thirdparties\%PLAT%\spd_log"

if not exist "build" mkdir "build"
if not exist "src\imgui" mkdir "src\imgui"

call :print_success "Directory structure created"
goto :eof

:setup_imgui
call :print_status "Setting up Dear ImGui..."

set "IMGUI_VERSION=v1.90.1"
set "IMGUI_URL=https://github.com/ocornut/imgui.git"
pushd "thirdparties\%PLAT%\imgui"

if not exist "imgui.cpp" (
    call :print_status "Downloading Dear ImGui %IMGUI_VERSION%..."
    
    REM Check if Git is available
    git --version >nul 2>&1
    if !errorlevel! neq 0 (
        call :print_error "Git is required but not found."
        popd
        exit /b 1
    )
    
    git clone --branch %IMGUI_VERSION% --depth 1 "%IMGUI_URL%" temp_imgui
    if !errorlevel! neq 0 (
        call :print_error "Failed to clone Dear ImGui repository"
        popd
        exit /b 1
    )
    
    REM Move contents from temp folder to current directory
    xcopy "temp_imgui\*" "." /E /Y >nul
    rmdir "temp_imgui" /S /Q
    
    call :print_success "Dear ImGui downloaded and extracted"
) else (
    call :print_warning "Dear ImGui already exists, skipping download"
)

popd
goto :eof

:setup_glfw
call :print_status "Setting up GLFW..."

set "GLFW_VERSION=3.4"
set "GLFW_URL=https://github.com/glfw/glfw.git"
pushd "thirdparties\%PLAT%\glfw"

if not exist "CMakeLists.txt" (
    call :print_status "Downloading GLFW %GLFW_VERSION%..."
    
    git clone --branch %GLFW_VERSION% --depth 1 "%GLFW_URL%" temp_glfw
    if !errorlevel! neq 0 (
        call :print_error "Failed to clone GLFW repository"
        popd
        exit /b 1
    )
    
    REM Move contents from temp folder to current directory
    xcopy "temp_glfw\*" "." /E /Y >nul
    rmdir "temp_glfw" /S /Q
    
    call :print_success "GLFW downloaded and extracted"
    
    REM Check if CMake is available
    cmake --version >nul 2>&1
    if !errorlevel! neq 0 (
        call :print_error "CMake is required to build GLFW."
        popd
        exit /b 1
    )
    
    call :print_status "Building GLFW %GLFW_VERSION%..."
    if not exist "build" mkdir "build"
    cmake -S . -B .\build -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>"
    if !errorlevel! neq 0 (
        call :print_error "CMake configuration failed"
        popd
        exit /b 1
    )
    
    cmake --build .\build --config Release
    if !errorlevel! neq 0 (
        call :print_error "GLFW build failed"
        popd
        exit /b 1
    )
    
    call :print_success "GLFW built successfully"
) else (
    call :print_warning "GLFW already exists, skipping download"
)

popd
goto :eof

:setup_gl3w
call :print_status "Setting up GL3W..."

pushd "thirdparties\%PLAT%\gl3w"

if not exist "src\gl3w.c" (
    call :print_status "Downloading GL3W..."
    
    REM Check if Git is available
    git --version >nul 2>&1
    if !errorlevel! neq 0 (
        call :print_error "Git is required to download GL3W."
        popd
        exit /b 1
    )
    
    REM Clone GL3W repository
    git clone --depth 1 https://github.com/skaslev/gl3w.git temp_gl3w
    if !errorlevel! neq 0 (
        call :print_error "Failed to clone GL3W repository."
        popd
        exit /b 1
    )
    
    pushd temp_gl3w
    
    REM Check if Python is available
    python3 --version >nul 2>&1
    if !errorlevel! equ 0 (
        call :print_status "Generating GL3W files with Python3..."
        python3 gl3w_gen.py
        if !errorlevel! neq 0 (
            call :print_error "Failed to generate GL3W files with Python3"
            popd
            popd
            exit /b 1
        )
    ) else (
        python --version >nul 2>&1
        if !errorlevel! equ 0 (
            call :print_status "Generating GL3W files with Python..."
            python gl3w_gen.py
            if !errorlevel! neq 0 (
                call :print_error "Failed to generate GL3W files with Python"
                popd
                popd
                exit /b 1
            )
        ) else (
            call :print_error "Python is required to generate GL3W files."
            popd
            popd
            exit /b 1
        )
    )
    
    popd
    
    REM Copy generated files
    if not exist "include" mkdir "include"
    if not exist "src" mkdir "src"
    xcopy "temp_gl3w\include" "include\" /E /I /Y >nul
    xcopy "temp_gl3w\src" "src\" /E /I /Y >nul
    rmdir "temp_gl3w" /S /Q
    
    call :print_success "GL3W generated and extracted"
) else (
    call :print_warning "GL3W already exists, skipping generation"
)

popd
goto :eof

:setup_spd_log
call :print_status "Setting up SPD_LOG..."

set "SPD_LOG_VERSION=v1.15.3"
set "SPD_URL=https://github.com/gabime/spdlog.git"
pushd "thirdparties\%PLAT%\spd_log"

if not exist "CMakeLists.txt" (
    call :print_status "Downloading SPD %SPD_LOG_VERSION%..."
    
    git clone --branch %SPD_LOG_VERSION% --depth 1 "%SPD_URL%" temp_spd
    if !errorlevel! neq 0 (
        call :print_error "Failed to clone SPD repository"
        popd
        exit /b 1
    )
    
    REM Move contents from temp folder to current directory
    xcopy "temp_spd\*" "." /E /Y >nul
    rmdir "temp_spd" /S /Q
    
    call :print_success "SPD downloaded and extracted"
    
    REM Check if CMake is available
    cmake --version >nul 2>&1
    if !errorlevel! neq 0 (
        call :print_error "CMake is required to build SPD."
        popd
        exit /b 1
    )
    
    call :print_status "Building SPD %SPD_VERSION%..."
    if not exist "build" mkdir "build"
    cmake -S . -B .\build -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>"
    if !errorlevel! neq 0 (
        call :print_error "CMake configuration failed"
        popd
        exit /b 1
    )
    
    cmake --build .\build --config Release
    if !errorlevel! neq 0 (
        call :print_error "SPD build failed"
        popd
        exit /b 1
    )
    
    call :print_success "SPD built successfully"
) else (
    call :print_warning "SPD already exists, skipping download"
)

popd
goto :eof

:main
@echo off
setlocal enabledelayedexpansion

echo Shape-Force - Dependency Setup (Windows)
echo ====================================================
echo.

call :detect_platform

REM Conditionally skip dependency check
if "%SKIP_DEP_CHECK%"=="1" (
    call :print_success "Skipping dependency check (SKIP_DEP_CHECK=1)"
) else (
    call :check_and_install_dependencies
    if !errorlevel! neq 0 exit /b 1
)

call :create_directories

call :print_status "Starting dependency download and setup..."

call :setup_imgui
if !errorlevel! neq 0 exit /b 1

call :setup_glfw
if !errorlevel! neq 0 exit /b 1

call :setup_gl3w
if !errorlevel! neq 0 exit /b 1

call :setup_spd_log
if !errorlevel! neq 0 exit /b 1

echo.
call :print_success "Dependency setup completed!"
call :print_status "You can now build the project with:"
echo   mkdir build ^&^& cd build
echo   cmake ..
echo   cmake --build . --config Release

exit /b 0