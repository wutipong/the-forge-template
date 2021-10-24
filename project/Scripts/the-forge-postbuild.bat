REM @ECHO OFF

SET SolutionDir=%1%
SET OutputDir=%2%

SET TheForgeDir=%SolutionDir%\the-forge
SET PythonExec=%TheForgeDir%\Tools\python-3.6.0-embed-amd64\python.exe
SET FslCompileExec= %PythonExec% %TheForgeDir%\Common_3\Tools\ForgeShadingLanguage\fsl.py

SET OutShader=%OutputDir%\Shaders
SET OutBinaryShader=%OutputDir%\CompiledShaders

SET CompileShader=%FslCompileExec% -d %OutShader% -b %OutBinaryShader% --compile -l "DIRECT3D11 DIRECT3D12 VULKAN" 

ECHO "Compiling ImGui shaders"
%CompileShader% %TheForgeDir%\Common_3\OS\UI\Shaders\FSL\imgui.frag.fsl
%CompileShader% %TheForgeDir%\Common_3\OS\UI\Shaders\FSL\imgui.vert.fsl
%CompileShader% %TheForgeDir%\Common_3\OS\UI\Shaders\FSL\textured_mesh.frag.fsl
%CompileShader% %TheForgeDir%\Common_3\OS\UI\Shaders\FSL\textured_mesh.vert.fsl

ECHO "Compiling FontStarsh shaders"
%CompileShader% %TheForgeDir%\Common_3\OS\Fonts\Shaders\FSL\fontstash.frag.fsl
%CompileShader% %TheForgeDir%\Common_3\OS\Fonts\Shaders\FSL\fontstash2D.vert.fsl
%CompileShader% %TheForgeDir%\Common_3\OS\Fonts\Shaders\FSL\fontstash3D.vert.fsl
