REM @ECHO OFF

SET SolutionDir=%1%
SET OutputDir=%2%
SET WindowsSdkVerBinPath=%3%

SET ProjectDir=%SolutionDir%project\

SET ScriptDir=%ProjectDir%Scripts\
SET CompileShaders=%ScriptDir%compile_shaders.py
SET PythonExec=python

SET FSL_COMPILER_FXC=%WindowsSdkVerBinPath%x64

ECHO compile shader files.
%PythonExec% %CompileShaders% -d %OutputDir%Shaders -b %OutputDir%CompiledShaders -s %SolutionDir% -w %WindowsSdkVerBinPath% %SolutionDir%the-forge\Common_3\OS\UI\Shaders\FSL
%PythonExec% %CompileShaders% -d %OutputDir%Shaders -b %OutputDir%CompiledShaders -s %SolutionDir% -w %WindowsSdkVerBinPath% %SolutionDir%the-forge\Common_3\OS\Fonts\Shaders\FSL
%PythonExec% %CompileShaders% -d %OutputDir%Shaders -b %OutputDir%CompiledShaders -s %SolutionDir% -w %WindowsSdkVerBinPath% %ProjectDir%Shaders

ECHO Copying texture files.
XCOPY %ProjectDir%Textures\ %OutputDir%Textures\ /Y /S

ECHO Copying meshes files
XCOPY %ProjectDir%Meshes\ %OutputDir%Meshes\ /Y /S

ECHO Copying font files.
XCOPY %ProjectDir%Fonts\ %OutputDir%Fonts\ /Y /S

ECHO Copying dll files.
XCOPY %ProjectDir%DLLs\ %OutputDir% /Y /S

ECHO Copying GPU config file
XCOPY %ProjectDir%GPUCfg\ %OutputDir%GPUCfg\ /Y /S