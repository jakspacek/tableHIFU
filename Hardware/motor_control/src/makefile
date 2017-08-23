# Run using 'nmake all' in the 32-bit "VS x86 Native Tools Command Prompt"
# Only supports 32-bit, since NI-FlexMotion is limited to 32-bit
# Make sure to only use 32-bit Python libraries and headers
# This isn't a well formatted makefile (I apologize), but it works

INCLUDE_PATH_PYTHON = "%UserProfile%\Miniconda2.32\envs\table_motors\include"
INCLUDE_PATH_FLEX = "C:\Program Files (x86)\National Instruments\NI-Motion\FlexMotion\Include"
INCLUDE_PATH_MOTION = MotionConsole

LIBRARY_PATH_PYTHON = "%UserProfile%\Miniconda2.32\envs\table_motors\libs"
LIBRARY_PATH_FLEX = "C:\Program Files (x86)\National Instruments\NI-Motion\FlexMotion\lib\Microsoft"

DEPS = $(INCLUDE_PATH_MOTION)\stdafx.cpp             \
       $(INCLUDE_PATH_MOTION)\NIMotionMotors.cpp     \
       $(INCLUDE_PATH_MOTION)\LogHandler.cpp         \
       $(INCLUDE_PATH_MOTION)\ConfigFile.cpp         \
       $(INCLUDE_PATH_MOTION)\Performance_Timers.cpp

EXTRA_LIBS = kernel32.lib \
             user32.lib   \
             gdi32.lib    \
             winspool.lib \
             comdlg32.lib \
             advapi32.lib \
             shell32.lib  \
             ole32.lib    \
             oleaut32.lib \
             uuid.lib     \
             odbc32.lib   \
             odbccp32.lib

motion_console_libraries: $(DEPS)
 cl /c /EHsc -Zi -D UNICODE /MD -I$(INCLUDE_PATH_FLEX) $(DEPS)
 lib stdafx.obj -LIBPATH:$(LIBRARY_PATH_FLEX)
 lib NIMotionMotors.obj -LIBPATH:$(LIBRARY_PATH_FLEX)
 lib LogHandler.obj -LIBPATH:$(LIBRARY_PATH_FLEX)
 lib ConfigFile.obj -LIBPATH:$(LIBRARY_PATH_FLEX)
 lib Performance_Timers.obj -LIBPATH:$(LIBRARY_PATH_FLEX)

python_library: motorcontrol.cpp
 cl /EHsc -Zi -D UNICODE /MD /LD -I$(INCLUDE_PATH_PYTHON) -I$(INCLUDE_PATH_FLEX) -I$(INCLUDE_PATH_MOTION) motorcontrol.cpp stdafx.lib NIMotionMotors.lib LogHandler.lib ConfigFile.lib Performance_Timers.lib FlexMS32.lib $(EXTRA_LIBS) /link /DLL /OUT:motorcontrol.pyd /verbose:lib -LIBPATH:$(LIBRARY_PATH_FLEX) -LIBPATH:$(LIBRARY_PATH_PYTHON)

clean:
 del *.obj
 del *.pdb
 del *.lib
 del *.exp
 del *.ilk
 del *.pyd.manifest
 if not exist "../lib" mkdir "../lib"
 move motorcontrol.pyd ../lib/motorcontrol.pyd

all: motion_console_libraries python_library clean
