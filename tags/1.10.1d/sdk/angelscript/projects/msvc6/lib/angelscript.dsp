# Microsoft Developer Studio Project File - Name="angelscript lib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=angelscript lib - Win32 Debug with ASM VM
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "angelscript.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "angelscript.mak" CFG="angelscript lib - Win32 Debug with ASM VM"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "angelscript lib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "angelscript lib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "angelscript lib - Win32 Debug with stats" (based on "Win32 (x86) Static Library")
!MESSAGE "angelscript lib - Win32 Release with ASM VM" (based on "Win32 (x86) Static Library")
!MESSAGE "angelscript lib - Win32 Debug with ASM VM" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "angelscript lib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /Za /W3 /O2 /Oy- /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "ANGELSCRIPT_EXPORT" /YX /FD /opt:nowin98 /c
# ADD BASE RSC /l 0x416 /d "NDEBUG"
# ADD RSC /l 0x416 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\lib\angelscript.lib"

!ELSEIF  "$(CFG)" == "angelscript lib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "ANGELSCRIPT_EXPORT" /YX /FD /opt:nowin98 /GZ /c
# ADD BASE RSC /l 0x416 /d "_DEBUG"
# ADD RSC /l 0x416 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\lib\angelscriptd.lib"

!ELSEIF  "$(CFG)" == "angelscript lib - Win32 Debug with stats"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "angelscript_lib___Win32_Debug_with_stats"
# PROP BASE Intermediate_Dir "angelscript_lib___Win32_Debug_with_stats"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_with_stats"
# PROP Intermediate_Dir "Debug_with_stats"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /ZI /Od /D "AS_DEBUG" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "ANGELSCRIPT_EXPORT" /YX /FD /opt:nowin98 /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /D "BUILD_WITH_LINE_CUES" /D "AS_DEBUG" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "ANGELSCRIPT_EXPORT" /YX /FD /opt:nowin98 /GZ /c
# ADD BASE RSC /l 0x416 /d "_DEBUG"
# ADD RSC /l 0x416 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\lib\angelscript.lib"
# ADD LIB32 /nologo /out:"..\..\..\lib\angelscriptd.lib"

!ELSEIF  "$(CFG)" == "angelscript lib - Win32 Release with ASM VM"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "angelscript_lib___Win32_Release_with_ASM_VM"
# PROP BASE Intermediate_Dir "angelscript_lib___Win32_Release_with_ASM_VM"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_asm_vm"
# PROP Intermediate_Dir "Release_asm_vm"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O2 /Oy- /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "ANGELSCRIPT_EXPORT" /YX /FD /opt:nowin98 /c
# ADD CPP /nologo /MD /W3 /O2 /Oy- /D "USE_ASM_VM" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "ANGELSCRIPT_EXPORT" /YX /FD /opt:nowin98 /c
# ADD BASE RSC /l 0x416 /d "NDEBUG"
# ADD RSC /l 0x416 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\lib\angelscript.lib"
# ADD LIB32 /nologo /out:"..\..\..\lib\angelscript.lib"

!ELSEIF  "$(CFG)" == "angelscript lib - Win32 Debug with ASM VM"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "angelscript_lib___Win32_Debug_with_ASM_VM"
# PROP BASE Intermediate_Dir "angelscript_lib___Win32_Debug_with_ASM_VM"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_asm_vm"
# PROP Intermediate_Dir "Debug_asm_vm"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "ANGELSCRIPT_EXPORT" /YX /FD /opt:nowin98 /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /D "USE_ASM_VM" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "ANGELSCRIPT_EXPORT" /YX /FD /opt:nowin98 /GZ /c
# ADD BASE RSC /l 0x416 /d "_DEBUG"
# ADD RSC /l 0x416 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\lib\angelscript.lib"
# ADD LIB32 /nologo /out:"..\..\..\lib\angelscriptd.lib"

!ENDIF 

# Begin Target

# Name "angelscript lib - Win32 Release"
# Name "angelscript lib - Win32 Debug"
# Name "angelscript lib - Win32 Debug with stats"
# Name "angelscript lib - Win32 Release with ASM VM"
# Name "angelscript lib - Win32 Debug with ASM VM"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\source\as_arrayobject.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_bstr_util.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_builder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_bytecode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_callfunc_x86.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_compiler.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_context.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_context_x86.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_datatype.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_map.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_module.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_objecttype.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_outputbuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_parser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_restore.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_scriptcode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_scriptengine.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_scriptfunction.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_scriptnode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_string.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_string_util.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_thread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_tokenizer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_typeinfo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_variablescope.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\source\as_array.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_arrayobject.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_bstr_util.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_builder.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_bytecode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_bytecodedef.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_callfunc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_compiler.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_config.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_context.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_datatype.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_debug.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_map.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_module.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_objecttype.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_outputbuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_parser.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_property.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_restore.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_scriptcode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_scriptengine.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_scriptfunction.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_scriptnode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_string.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_string_util.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_structs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_texts.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_thread.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_tokendef.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_tokenizer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_typeinfo.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_types.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\as_variablescope.h
# End Source File
# End Group
# Begin Group "Interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\include\angelscript.h
# End Source File
# End Group
# End Target
# End Project