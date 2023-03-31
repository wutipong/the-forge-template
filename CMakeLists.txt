cmake_minimum_required(VERSION 3.17)

set(TARGET_NAME the-forge-template)

project(${TARGET_NAME})

if(WIN32)
    set(Python3_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/The-Forge/Tools/python-3.6.0-embed-amd64")
endif()

FIND_PACKAGE(Python3 3.6 EXACT COMPONENTS Interpreter)

add_executable(${TARGET_NAME}
        "MainApp.cpp"
        "DemoScene.cpp"
        "SceneManagement.cpp"
        "The-Forge/Common_3/Application/CameraController.cpp"
        "The-Forge/Common_3/Application/Fonts/FontSystem.cpp"
        "The-Forge/Common_3/Application/Fonts/stbtt.cpp"
        "The-Forge/Common_3/Application/InputSystem.cpp"
        "The-Forge/Common_3/Application/Profiler/GpuProfiler.cpp"
        "The-Forge/Common_3/Application/Profiler/ProfilerBase.cpp"
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
        "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/hid/hidparsers/HIDParserPS5Controller.cpp"
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
        "The-Forge/Common_3/Game/Scripting/LuaSystem.cpp"
        "The-Forge/Common_3/Graphics/CommonShaderReflection.cpp"
        "The-Forge/Common_3/Graphics/DependencyTracking.cpp"
        "The-Forge/Common_3/Graphics/Direct3D11/Direct3D11.cpp"
        "The-Forge/Common_3/Graphics/Direct3D11/Direct3D11Raytracing.cpp"
        "The-Forge/Common_3/Graphics/Direct3D11/Direct3D11ShaderReflection.cpp"
        "The-Forge/Common_3/Graphics/Direct3D12/Direct3D12.cpp"
        "The-Forge/Common_3/Graphics/Direct3D12/Direct3D12Hooks.cpp"
        "The-Forge/Common_3/Graphics/Direct3D12/Direct3D12Raytracing.cpp"
        "The-Forge/Common_3/Graphics/Direct3D12/Direct3D12ShaderReflection.cpp"
        "The-Forge/Common_3/Graphics/GPUConfig.cpp"
        "The-Forge/Common_3/Graphics/Graphics.cpp"
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
        "The-Forge/Common_3/Utilities/FileSystem/FileSystem.cpp"
        "The-Forge/Common_3/Utilities/Log/Log.c"
        "The-Forge/Common_3/Utilities/Math/StbDs.c"
        "The-Forge/Common_3/Utilities/MemoryTracking/MemoryTracking.c"
        "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/bstrlib/bstrlib.c"
        "The-Forge/Common_3/Utilities/Timer.c"
        "The-Forge/Common_3/Utilities/Math/Algorithms.c"
        "The-Forge/Common_3/Game/Scripting/LuaManagerImpl.cpp"
        "The-Forge/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/SpirvTools.cpp"
        "The-Forge/Common_3/Utilities/FileSystem/SystemRun.cpp"
        "The-Forge/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/meshoptimizer/src/allocator.cpp"
        "The-Forge/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/meshoptimizer/src/clusterizer.cpp"
        "The-Forge/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/meshoptimizer/src/indexcodec.cpp"
        "The-Forge/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/meshoptimizer/src/indexgenerator.cpp"
        "The-Forge/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/meshoptimizer/src/overdrawanalyzer.cpp"
        "The-Forge/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/meshoptimizer/src/overdrawoptimizer.cpp"
        "The-Forge/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/meshoptimizer/src/simplifier.cpp"
        "The-Forge/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/meshoptimizer/src/spatialorder.cpp"
        "The-Forge/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/meshoptimizer/src/stripifier.cpp"
        "The-Forge/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/meshoptimizer/src/vcacheanalyzer.cpp"
        "The-Forge/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/meshoptimizer/src/vcacheoptimizer.cpp"
        "The-Forge/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/meshoptimizer/src/vertexcodec.cpp"
        "The-Forge/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/meshoptimizer/src/vertexfilter.cpp"
        "The-Forge/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/meshoptimizer/src/vfetchanalyzer.cpp"
        "The-Forge/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/meshoptimizer/src/vfetchoptimizer.cpp"
        "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/basis_universal/transcoder/basisu_transcoder.cpp"
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
        Demo2Scene.cpp Demo2Scene.h ShapeDrawer.cpp ShapeDrawer.h)

find_package(Vulkan REQUIRED)

target_compile_features(${TARGET_NAME} PRIVATE cxx_std_20)
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

if (MSVC)
    # set_property(TARGET the-forge-template PROPERTY WIN32_EXECUTABLE ON)
    target_compile_options(${TARGET_NAME} PRIVATE /Zc:__cplusplus)
endif ()

target_include_directories(${TARGET_NAME} PRIVATE
        "The-Forge/Common_3/Application/Interfaces"
        "The-Forge/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source"
        "The-Forge/Common_3/Graphics/Interfaces"
        "The-Forge/Common_3/OS/Interfaces"
        "The-Forge/Common_3/Resources/ResourceLoader/Interfaces"
        "The-Forge/Common_3/Utilities"
        "The-Forge/Common_3/Utilities/Interfaces"
        "The-Forge/Common_3/Utilities/ThirdParty/OpenSource/Nothings/"
        ${VULKAN_HEADERS_INCLUDE_DIRS}
        )

target_link_directories(${TARGET_NAME} PRIVATE
        "The-Forge/Common_3/OS/ThirdParty/OpenSource/winpixeventruntime/bin"
        "The-Forge/Common_3/Graphics/ThirdParty/OpenSource/ags/ags_lib/lib"
        "The-Forge/Common_3/Graphics/ThirdParty/OpenSource/nvapi/amd64"
        )

target_link_libraries(${TARGET_NAME} PRIVATE XInput Vulkan::Vulkan WinPixEventRuntime amd_ags_x64 nvapi64)

file(COPY
        "${CMAKE_CURRENT_SOURCE_DIR}/The-Forge/Common_3/OS/ThirdParty/OpenSource/winpixeventruntime/bin/WinPixEventRuntime.dll"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

file(COPY
        "${CMAKE_CURRENT_SOURCE_DIR}/The-Forge/Common_3/Graphics/ThirdParty/OpenSource/ags/ags_lib/lib/amd_ags_x64.dll"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

file(COPY
        "${CMAKE_CURRENT_SOURCE_DIR}/The-Forge/Examples_3/Unit_Tests/UnitTestResources/GPUCfg/gpu.cfg"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/GPUCfg")

file(COPY
        "${CMAKE_CURRENT_SOURCE_DIR}/The-Forge/Examples_3/Unit_Tests/UnitTestResources/Fonts/TitilliumText/TitilliumText-Bold.otf"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Fonts/TitilliumText")

function(compile_shaders)
    set(oneValueArgs TARGET SHADER_LIST)
    cmake_parse_arguments(COMPILE_SHADERS "${options}" "${oneValueArgs}"
            "${multiValueArgs}" ${ARGN}
            )

    add_custom_target(${COMPILE_SHADERS_TARGET}
            COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/The-Forge/Common_3/Tools/ForgeShadingLanguage/fsl.py"
            "-dShaders" "-bCompiledShaders" "-l DIRECT3D12 VULKAN" --compile --verbose
            ${COMPILE_SHADERS_SHADER_LIST}
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            )
    add_dependencies(${TARGET_NAME} ${COMPILE_SHADERS_TARGET})
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