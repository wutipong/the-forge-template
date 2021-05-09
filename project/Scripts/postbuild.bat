@ECHO OFF

SET ProjectDir=%1%
SET OutputDir=%2%

ECHO Copying shader files.
XCOPY %ProjectDir%Shaders\ %OutputDir%Shaders\ /L /Y /S

ECHO Copying texture files.
XCOPY %ProjectDir%Textures\ %OutputDir%Textures\ /L /Y /S

ECHO Copying fonts files.
XCOPY %ProjectDir%Fonts\ %OutputDir%Fonts\ /L /Y /S