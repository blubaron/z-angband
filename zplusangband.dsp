# Microsoft Developer Studio Project File - Name="zplusangband" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=zplusangband - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "zplusangband.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zplusangband.mak" CFG="zplusangband - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zplusangband - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "zplusangband - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "zplusangband - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "src" /I "src/platform/win" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 src\lua\tolua.lib d3d9.lib d3dx9.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"zplusangband.exe"

!ELSEIF  "$(CFG)" == "zplusangband - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "src" /I "src/platform/win" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 src\lua\tolua.lib d3d9.lib d3dx9.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"zplusangband_d.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "zplusangband - Win32 Release"
# Name "zplusangband - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\artifact.c
# End Source File
# Begin Source File

SOURCE=.\src\avatar.c
# End Source File
# Begin Source File

SOURCE=.\src\birth.c
# End Source File
# Begin Source File

SOURCE=.\src\bldg.c
# End Source File
# Begin Source File

SOURCE=.\src\button.c
# End Source File
# Begin Source File

SOURCE=.\src\cave.c
# End Source File
# Begin Source File

SOURCE=.\src\cmd-context.c
# End Source File
# Begin Source File

SOURCE=.\src\cmd0.c
# End Source File
# Begin Source File

SOURCE=.\src\cmd1.c
# End Source File
# Begin Source File

SOURCE=.\src\cmd2.c
# End Source File
# Begin Source File

SOURCE=.\src\cmd3.c
# End Source File
# Begin Source File

SOURCE=.\src\cmd4.c
# End Source File
# Begin Source File

SOURCE=.\src\cmd5.c
# End Source File
# Begin Source File

SOURCE=.\src\cmd6.c
# End Source File
# Begin Source File

SOURCE=.\src\cmd7.c
# End Source File
# Begin Source File

SOURCE=.\src\dungeon.c
# End Source File
# Begin Source File

SOURCE=.\src\effects.c
# End Source File
# Begin Source File

SOURCE=.\src\fields.c
# End Source File
# Begin Source File

SOURCE=.\src\files.c
# End Source File
# Begin Source File

SOURCE=.\src\flavor.c
# End Source File
# Begin Source File

SOURCE=.\src\game.c
# End Source File
# Begin Source File

SOURCE=.\src\generate.c
# End Source File
# Begin Source File

SOURCE=.\src\grafmode.c
# End Source File
# Begin Source File

SOURCE=.\src\grid.c
# End Source File
# Begin Source File

SOURCE=.\src\hero.c
# End Source File
# Begin Source File

SOURCE=.\src\init1.c
# End Source File
# Begin Source File

SOURCE=.\src\init2.c
# End Source File
# Begin Source File

SOURCE=".\src\l-field.c"
# End Source File
# Begin Source File

SOURCE=".\src\l-misc.c"
# End Source File
# Begin Source File

SOURCE=".\src\l-monst.c"
# End Source File
# Begin Source File

SOURCE=".\src\l-object.c"
# End Source File
# Begin Source File

SOURCE=".\src\l-player.c"
# End Source File
# Begin Source File

SOURCE=".\src\l-random.c"
# End Source File
# Begin Source File

SOURCE=".\src\l-spell.c"
# End Source File
# Begin Source File

SOURCE=".\src\l-ui.c"
# End Source File
# Begin Source File

SOURCE=.\src\load.c
# End Source File
# Begin Source File

SOURCE=".\src\maid-grf.c"
# End Source File
# Begin Source File

SOURCE=".\src\platform\win\main-win.c"
# End Source File
# Begin Source File

SOURCE=.\src\melee1.c
# End Source File
# Begin Source File

SOURCE=.\src\melee2.c
# End Source File
# Begin Source File

SOURCE=.\src\mind.c
# End Source File
# Begin Source File

SOURCE=.\src\monster1.c
# End Source File
# Begin Source File

SOURCE=.\src\monster2.c
# End Source File
# Begin Source File

SOURCE=.\src\mspells1.c
# End Source File
# Begin Source File

SOURCE=.\src\mspells2.c
# End Source File
# Begin Source File

SOURCE=.\src\mutation.c
# End Source File
# Begin Source File

SOURCE=.\src\notes.c
# End Source File
# Begin Source File

SOURCE=.\src\obj_kind.c
# End Source File
# Begin Source File

SOURCE=.\src\object1.c
# End Source File
# Begin Source File

SOURCE=.\src\object2.c
# End Source File
# Begin Source File

SOURCE=.\src\quest1.c
# End Source File
# Begin Source File

SOURCE=.\src\quest2.c
# End Source File
# Begin Source File

SOURCE=.\src\racial.c
# End Source File
# Begin Source File

SOURCE=.\src\platform\win\readdib.c
# End Source File
# Begin Source File

SOURCE=.\src\platform\win\readdx9.c
# End Source File
# Begin Source File

SOURCE=.\src\rooms.c
# End Source File
# Begin Source File

SOURCE=.\src\run.c
# End Source File
# Begin Source File

SOURCE=.\src\save.c
# End Source File
# Begin Source File

SOURCE=.\src\scores.c
# End Source File
# Begin Source File

SOURCE=.\src\script.c
# End Source File
# Begin Source File

SOURCE=.\src\signals.c
# End Source File
# Begin Source File

SOURCE=.\src\spells1.c
# End Source File
# Begin Source File

SOURCE=.\src\spells2.c
# End Source File
# Begin Source File

SOURCE=.\src\spells3.c
# End Source File
# Begin Source File

SOURCE=.\src\store.c
# End Source File
# Begin Source File

SOURCE=.\src\streams.c
# End Source File
# Begin Source File

SOURCE=.\src\tables.c
# End Source File
# Begin Source File

SOURCE=.\src\ui.c
# End Source File
# Begin Source File

SOURCE=.\src\ui-inkey.c
# End Source File
# Begin Source File

SOURCE=.\src\ui-menu.c
# End Source File
# Begin Source File

SOURCE=.\src\ui-region.c
# End Source File
# Begin Source File

SOURCE=.\src\userid.c
# End Source File
# Begin Source File

SOURCE=.\src\util.c
# End Source File
# Begin Source File

SOURCE=.\src\variable.c
# End Source File
# Begin Source File

SOURCE=.\src\wild1.c
# End Source File
# Begin Source File

SOURCE=.\src\wild2.c
# End Source File
# Begin Source File

SOURCE=.\src\wild3.c
# End Source File
# Begin Source File

SOURCE=.\src\wild4.c
# End Source File
# Begin Source File

SOURCE=".\src\platform\win\win-layout.c"
# End Source File
# Begin Source File

SOURCE=.\src\wizard1.c
# End Source File
# Begin Source File

SOURCE=.\src\wizard2.c
# End Source File
# Begin Source File

SOURCE=.\src\xtra1.c
# End Source File
# Begin Source File

SOURCE=.\src\xtra2.c
# End Source File
# Begin Source File

SOURCE=.\src\z-file.c
# End Source File
# Begin Source File

SOURCE=".\src\z-form.c"
# End Source File
# Begin Source File

SOURCE=.\src\z-msg.c
# End Source File
# Begin Source File

SOURCE=.\src\z-quark.c
# End Source File
# Begin Source File

SOURCE=".\src\z-rand.c"
# End Source File
# Begin Source File

SOURCE=".\src\z-term.c"
# End Source File
# Begin Source File

SOURCE=".\src\z-util.c"
# End Source File
# Begin Source File

SOURCE=".\src\z-virt.c"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\angband.h
# End Source File
# Begin Source File

SOURCE=.\src\autoconf.h
# End Source File
# Begin Source File

SOURCE=.\src\button.h
# End Source File
# Begin Source File

SOURCE=.\src\defines.h
# End Source File
# Begin Source File

SOURCE=.\src\externs.h
# End Source File
# Begin Source File

SOURCE=.\src\game.h
# End Source File
# Begin Source File

SOURCE=.\src\generate.h
# End Source File
# Begin Source File

SOURCE=.\src\grafmode.h
# End Source File
# Begin Source File

SOURCE=.\src\grid.h
# End Source File
# Begin Source File

SOURCE=".\src\h-basic.h"
# End Source File
# Begin Source File

SOURCE=".\src\h-config.h"
# End Source File
# Begin Source File

SOURCE=".\src\h-define.h"
# End Source File
# Begin Source File

SOURCE=".\src\h-system.h"
# End Source File
# Begin Source File

SOURCE=".\src\h-type.h"
# End Source File
# Begin Source File

SOURCE=.\src\init.h
# End Source File
# Begin Source File

SOURCE=.\src\lua.h
# End Source File
# Begin Source File

SOURCE=".\src\maid-grf.h"
# End Source File
# Begin Source File

SOURCE=.\src\options.h
# End Source File
# Begin Source File

SOURCE=.\src\platform\win\readdib.h
# End Source File
# Begin Source File

SOURCE=.\src\rooms.h
# End Source File
# Begin Source File

SOURCE=.\src\script.h
# End Source File
# Begin Source File

SOURCE=".\src\signals.h"
# End Source File
# Begin Source File

SOURCE=.\src\streams.h
# End Source File
# Begin Source File

SOURCE=.\src\tvalsval.h
# End Source File
# Begin Source File

SOURCE=.\src\types.h
# End Source File
# Begin Source File

SOURCE=".\src\ui.h"
# End Source File
# Begin Source File

SOURCE=".\src\ui-inkey.h"
# End Source File
# Begin Source File

SOURCE=".\src\ui-menu.h"
# End Source File
# Begin Source File

SOURCE=".\src\ui-region.h"
# End Source File
# Begin Source File

SOURCE=".\src\userid.h"
# End Source File
# Begin Source File

SOURCE=".\src\util.h"
# End Source File
# Begin Source File

SOURCE=.\src\version.h
# End Source File
# Begin Source File

SOURCE=.\src\wild.h
# End Source File
# Begin Source File

SOURCE=".\src\platform\win\win-menu.h"
# End Source File
# Begin Source File

SOURCE=".\src\platform\win\win-term.h"
# End Source File
# Begin Source File

SOURCE=".\src\z-config.h"
# End Source File
# Begin Source File

SOURCE=".\src\z-file.h"
# End Source File
# Begin Source File

SOURCE=".\src\z-form.h"
# End Source File
# Begin Source File

SOURCE=".\src\z-msg.h"
# End Source File
# Begin Source File

SOURCE=".\src\z-quark.h"
# End Source File
# Begin Source File

SOURCE=".\src\z-rand.h"
# End Source File
# Begin Source File

SOURCE=".\src\z-term.h"
# End Source File
# Begin Source File

SOURCE=".\src\z-util.h"
# End Source File
# Begin Source File

SOURCE=".\src\z-virt.h"
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\src\platform\win\zplus.rc
# End Source File
# End Group
# End Target
# End Project
