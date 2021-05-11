@ECHO OFF

SET ProjectDir=%1%
SET OutputDir=%2%

ECHO Copying shader files.
XCOPY %ProjectDir%Shaders\ %OutputDir%Shaders\ /Y /S

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