# the-forge-template

This is a CMake-based template project for creating project with [ConfettifFX's The-Forge](https://github.com/ConfettiFX/The-Forge). 
Currently Windows and Manjaro Linux is tested.

This template has C++20 flags enabled.

## Requirements

### All Platforms

* CMake version 3.19 or higher

### Windows

* Visual Studio 2019 with C++ Desktop Development components installed.
  * VS 2019 Build Tools also work.
  * VS 2017 might work but it's untested.

### Manjaro

* Clang 16.0.6 is tested
* libc++ 16.0.6 is tested.
* Python 3.6. This can be installed by installing the package `python36` from AUR repository.
* pkg-config.

**note:** libstdc++ which is the standard C++ library of GCC does not work with this project due to how The-Forge tries
to disable `delete` operator in some header files. Libstdc++ delete some constructor when the C++ standard flag is
higher than a specific value thus this becomes problematic. 

libc++ is choosen here as a workaround as its standard library does not contain such deleted constructor.

## Building

### Windows

On Windows, just following the common CMake workflow.  

### Linux
On Linux, an extra step is required as one of the executable included are not atrributed correctly. We have to
make the `glslangValidator` becomes executable by chmod it.

For example:

```sh
chmod +x "./The-Forge/Common_3/Graphics/ThirdParty/OpenSource/VulkanSDK/bin/Linux/glslangValidator"
```

## How it works

In the `CMakeList.txt`, there are 2 library and a executable targets. The libraries contains codes from **The-Forge**
only. The executable target name **main** can be updated to suite the needs of the user.

The reason why there are two libraries is one of the third-party libraries -- namely **gainput**-- is expected to be
Multibyte Charactor Set on Windows, while the rest of The-Forge is Unicode Chractor Set. This cause conflicts
of the preprocessor/compiler flags within the code base. Making these separated library is necessary.

The demo projects shipped with The-Forge has depedencies setup as separated projects to the core libray. However,
the template put all of the codes into one library project (except **gainput** as discussed earlier).

Also there is a function `compile_shaders` that creates a target that compiles shaders programs. The user only adds
more of this function calls when they want to include shaders from the library in thir project. For adding shaders,
just make changes to the files in the **shaders** directory. 