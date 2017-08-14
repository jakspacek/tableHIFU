@echo off
echo %cd%

if not exist miniconda_path.txt (
    echo The file 'miniconda_path.txt' does not exist. Exiting.
    pause
    exit
)

set /p MINICONDA_PATH=< miniconda_path.txt
if %errorlevel% neq 0 (
    echo Error while reading the 'miniconda_path.txt' file. Exiting.
    pause
    exit
)

if ["%MINICONDA_PATH%"] == [] (
    echo The Miniconda path was empty: %MINICONDA_PATH%
    echo Exiting.
    pause
    exit
)

echo 32-bit Miniconda path is set to: %MINICONDA_PATH%
set PATH=%MINICONDA_PATH%;%MINICONDA_PATH%\Scripts;%MINICONDA_PATH%\Library\bin;%PATH%
if %errorlevel% neq 0 (
    pause
    exit
)

if %errorlevel% neq 0 (
    pause
    exit
)

call activate HIFU_table
if %errorlevel% neq 0 (
    echo Cannot activate the conda environment 'HIFU_table'. Exiting.
    pause
    exit
)

echo Starting the server.
echo.
python HIFU_server.py
if %errorlevel% neq 0 (
    pause
    exit
)