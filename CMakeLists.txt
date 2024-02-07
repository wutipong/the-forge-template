cmake_minimum_required(VERSION 3.17)

project(the-forge-template)

add_executable(main
    "MainApp.cpp"
    "DemoScene.cpp" "DemoScene.h"
    "Demo2Scene.cpp" "Demo2Scene.h" 
    "DrawShape.cpp" "DrawShape.h"
    "Drawquad.cpp" "DrawQuad.h"
    "QuadDemoScene.cpp" "QuadDemoScene.h"
    # "SMAA.cpp"  "SMAA.h"
    "ColorGrading.cpp" "ColorGrading.h"
    "PostProcessing.cpp" "PostProcessing.h"
    "The-Forge/Common_3/Application/CameraController.cpp"
    "The-Forge/Common_3/Application/Fonts/FontSystem.cpp"
    "The-Forge/Common_3/Application/Fonts/stbtt.cpp"
    "The-Forge/Common_3/Application/InputSystem.cpp"
    "The-Forge/Common_3/Application/Profiler/GpuProfiler.cpp"
    "The-Forge/Common_3/Application/Profiler/ProfilerBase.cpp"
    "The-Forge/Common_3/Application/RemoteControl.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/gainput.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/GainputAllocator.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/GainputInputDeltaState.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/GainputInputDevice.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/GainputInputManager.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/GainputInputMap.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/GainputInputState.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/GainputMapFilters.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/hid/GainputHID.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/hid/GainputHIDWhitelist.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/hid/hidparsers/HIDParserPS4Controller.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/hid/hidparsers/HIDParserPS4Controller.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/hid/hidparsers/HIDParserPS4Controller.h"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/hid/hidparsers/HIDParserPS5Controller.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/hid/hidparsers/HIDParserPS5Controller.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/hid/hidparsers/HIDParserPS5Controller.h"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/hid/hidparsers/HIDParserSwitchController.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/hid/hidparsers/HIDParserSwitchController.h"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/keyboard/GainputInputDeviceKeyboard.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/mouse/GainputInputDeviceMouse.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/pad/GainputInputDevicePad.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/touch/GainputInputDeviceTouch.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/hidapi/windows/hid.c"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/imgui/imgui_demo.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/imgui/imgui_draw.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/imgui/imgui_tables.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/imgui/imgui_widgets.cpp"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/imgui/imgui.cpp"
    "The-Forge/Common_3/Application/UI/UI.cpp"
    "The-Forge/Common_3/Game/Scripting/LuaManager.cpp"
    "The-Forge/Common_3/Game/Scripting/LuaManagerImpl.cpp"
    "The-Forge/Common_3/Game/Scripting/LuaSystem.cpp"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lapi.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lauxlib.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lbaselib.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lbitlib.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lcode.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lcorolib.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lctype.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/ldblib.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/ldebug.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/ldo.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/ldump.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lfunc.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lgc.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/linit.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/liolib.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/llex.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lmathlib.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lmem.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/loadlib.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lobject.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lopcodes.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/loslib.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lparser.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lstate.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lstring.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lstrlib.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/ltable.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/ltablib.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/ltm.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lundump.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lutf8lib.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lvm.c"
    "The-Forge/Common_3/Game/ThirdParty/OpenSource/lua-5.3.5/src/lzio.c"
    "The-Forge/Common_3/Graphics/CommonShaderReflection.cpp"
    "The-Forge/Common_3/Graphics/Direct3D11/Direct3D11.cpp"
    "The-Forge/Common_3/Graphics/Direct3D11/Direct3D11ShaderReflection.cpp"
    "The-Forge/Common_3/Graphics/Direct3D12/Direct3D12.cpp"
    "The-Forge/Common_3/Graphics/Direct3D12/Direct3D12Hooks.cpp"
    "The-Forge/Common_3/Graphics/Direct3D12/Direct3D12Raytracing.cpp"
    "The-Forge/Common_3/Graphics/Direct3D12/Direct3D12ShaderReflection.cpp"
    "The-Forge/Common_3/Graphics/GPUConfig.cpp"
    "The-Forge/Common_3/Graphics/PickRenderingAPI.cpp"
    "The-Forge/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/spirv_cfg.cpp"
    "The-Forge/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/spirv_cpp.cpp"
    "The-Forge/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/spirv_cross_parsed_ir.cpp"
    "The-Forge/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/spirv_cross_util.cpp"
    "The-Forge/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/spirv_cross.cpp"
    "The-Forge/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/spirv_glsl.cpp"
    "The-Forge/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/spirv_hlsl.cpp"
    "The-Forge/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/spirv_msl.cpp"
    "The-Forge/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/spirv_parser.cpp"
    "The-Forge/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/spirv_reflect.cpp"
    "The-Forge/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/SpirvTools.cpp"
    "The-Forge/Common_3/Graphics/Vulkan/Vulkan.cpp"
    "The-Forge/Common_3/Graphics/Vulkan/VulkanRaytracing.cpp"
    "The-Forge/Common_3/Graphics/Vulkan/VulkanShaderReflection.cpp"
    "The-Forge/Common_3/OS/CPUConfig.cpp"
    "The-Forge/Common_3/OS/ThirdParty/OpenSource/cpu_features/src/impl_x86_windows.c"
    "The-Forge/Common_3/OS/Windows/WindowsBase.cpp"
    "The-Forge/Common_3/OS/Windows/WindowsFileSystem.cpp"
    "The-Forge/Common_3/OS/Windows/WindowsLog.c"
    "The-Forge/Common_3/OS/Windows/WindowsStackTraceDump.cpp"
    "The-Forge/Common_3/OS/Windows/WindowsThread.c"
    "The-Forge/Common_3/OS/Windows/WindowsTime.c"
    "The-Forge/Common_3/OS/Windows/WindowsToolsFileSystem.cpp"
    "The-Forge/Common_3/OS/Windows/WindowsWindow.cpp"
    "The-Forge/Common_3/OS/WindowSystem/WindowSystem.cpp"
    "The-Forge/Common_3/Resources/ResourceLoader/ResourceLoader.cpp"
    "The-Forge/Common_3/Tools/Network/Network.c"
    "The-Forge/Common_3/Tools/ReloadServer/ReloadClient.cpp"
    "The-Forge/Common_3/Utilities/FileSystem/FileSystem.c"
    "The-Forge/Common_3/Utilities/FileSystem/SystemRun.c"
    "The-Forge/Common_3/Utilities/Log/Log.c"
    "The-Forge/Common_3/Utilities/Math/Algorithms.c"
    "The-Forge/Common_3/Utilities/Math/StbDs.c"
    "The-Forge/Common_3/Utilities/MemoryTracking/MemoryTracking.c"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/basis_universal/transcoder/basisu_transcoder.cpp"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/bstrlib/bstrlib.c"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/lz4/lz4.c"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/lz4/lz4.h"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/lz4/lz4hc.c"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/lz4/lz4hc.h"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/bits.h"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/bitstream.h"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/compiler.h"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/cpu.h"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/debug.c"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/debug.h"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/entropy_common.c"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/error_private.c"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/error_private.h"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/fse_decompress.c"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/fse.h"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/huf.h"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/mem.h"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/pool.c"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/pool.h"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/portability_macros.h"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/threading.c"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/threading.h"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/xxhash.c"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/xxhash.h"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/zstd_common.c"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/zstd_deps.h"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/zstd_internal.h"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/common/zstd_trace.h"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/decompress/huf_decompress.c"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/decompress/zstd_ddict.c"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/decompress/zstd_ddict.h"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/decompress/zstd_decompress_block.c"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/decompress/zstd_decompress_block.h"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/decompress/zstd_decompress_internal.h"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/zstd/decompress/zstd_decompress.c"
    "The-Forge/Common_3/Utilities/Timer.c"
)

# find_package(Vulkan REQUIRED)

target_compile_features(main PRIVATE cxx_std_20)
set_property(TARGET main PROPERTY CXX_STANDARD 20)
set_property(TARGET main PROPERTY CXX_STANDARD_REQUIRED ON)

if (MSVC)
    # set_property(TARGET main PROPERTY WIN32_EXECUTABLE ON)
    target_compile_options(main PRIVATE /Zc:__cplusplus)
    target_compile_definitions (main PRIVATE -D_UNICODE -DUNICODE)
endif ()

target_include_directories(main PRIVATE
    "The-Forge/Common_3/Application/Interfaces"
    "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source"
    "The-Forge/Common_3/Graphics/Interfaces"
    "The-Forge/Common_3/Graphics/ThirdParty/OpenSource/DirectXShaderCompiler/inc"
    "The-Forge/Common_3/OS/Interfaces"
    "The-Forge/Common_3/Resources/ResourceLoader/Interfaces"
    "The-Forge/Common_3/Utilities"
    "The-Forge/Common_3/Utilities/Interfaces"
    "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/Nothings/"
    ${VULKAN_HEADERS_INCLUDE_DIRS}
)

target_link_directories(main PRIVATE
    "The-Forge/Common_3/Graphics/ThirdParty/OpenSource/ags/ags_lib/lib"
    "The-Forge/Common_3/Graphics/ThirdParty/OpenSource/DirectXShaderCompiler/lib/x64"
    "The-Forge/Common_3/Graphics/ThirdParty/OpenSource/nvapi/amd64"
    "The-Forge/Common_3/OS/ThirdParty/OpenSource/winpixeventruntime/bin"
)

target_link_libraries(main PRIVATE 
    XInput 
#    Vulkan::Vulkan 
    WinPixEventRuntime 
    amd_ags_x64 
    nvapi64
    dxcompiler
)

add_custom_target(CopyDlls ALL      
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:main>/"      
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/The-Forge/Common_3/OS/ThirdParty/OpenSource/winpixeventruntime/bin/WinPixEventRuntime.dll" "$<TARGET_FILE_DIR:main>/"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/ags/ags_lib/lib/amd_ags_x64.dll" "$<TARGET_FILE_DIR:main>/"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/DirectXShaderCompiler/bin/x64/dxcompiler.dll" "$<TARGET_FILE_DIR:main>/"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/DirectXShaderCompiler/bin/x64/dxil.dll" "$<TARGET_FILE_DIR:main>/"
)

add_custom_target(CopyGpuCfg ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:main>/GPUCfg/"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/The-Forge/Examples_3/Unit_Tests/src/01_Transformations/GPUCfg/gpu.cfg" "$<TARGET_FILE_DIR:main>/GPUCfg/"
)

add_custom_target(CopyFonts ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:main>/Fonts/TitilliumText/"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/The-Forge/Examples_3/Unit_Tests/UnitTestResources/Fonts/TitilliumText/TitilliumText-Bold.otf" "$<TARGET_FILE_DIR:main>/Fonts/TitilliumText/"
)

add_custom_target(CopyTexture ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:main>/Textures/"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Textures/Quad0.dds" "$<TARGET_FILE_DIR:main>/Textures/"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Textures/Quad1.dds" "$<TARGET_FILE_DIR:main>/Textures/"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Textures/LUT_0.dds" "$<TARGET_FILE_DIR:main>/Textures/"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Textures/LUT_1.dds" "$<TARGET_FILE_DIR:main>/Textures/"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Textures/LUT_2.dds" "$<TARGET_FILE_DIR:main>/Textures/"
)

if(WIN32)
    set(Python3_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/The-Forge/Tools/python-3.6.0-embed-amd64")
endif()

find_package(Python3 3.6 EXACT COMPONENTS Interpreter)

function(compile_shaders)
    set(oneValueArgs TARGET SHADER_LIST)
    cmake_parse_arguments(COMPILE_SHADERS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_custom_target(${COMPILE_SHADERS_TARGET} ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:main>/Shaders"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:main>/CompiledShaders"
        COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/The-Forge/Common_3/Tools/ForgeShadingLanguage/fsl.py"
            "-dShaders" "-bCompiledShaders" "-l DIRECT3D12 VULKAN" --compile --verbose
            ${COMPILE_SHADERS_SHADER_LIST}
            WORKING_DIRECTORY "$<TARGET_FILE_DIR:main>"
)
endfunction()

compile_shaders(
    TARGET UiShader
    SHADER_LIST "${CMAKE_CURRENT_SOURCE_DIR}/The-Forge/Common_3/Application/UI/Shaders/FSL/UI_ShaderList.fsl"
)

compile_shaders(
    TARGET FontShader
    SHADER_LIST "${CMAKE_CURRENT_SOURCE_DIR}/The-Forge/Common_3/Application/Fonts/Shaders/FSL/Fonts_ShaderList.fsl"
)

compile_shaders(
    TARGET ProjectShader
    SHADER_LIST "${CMAKE_CURRENT_SOURCE_DIR}/shaders/FSL/ShaderList.fsl"
)