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
* Python 3. 3.6.9, 3.11.8 are tested.
* pkg-config.

**note:** libstdc++ which is the standard C++ library of GCC does not work with this project due to how The-Forge tries
to disable `delete` operator in some header files. Libstdc++ delete some constructor when the C++ standard flag is
higher than a specific value thus this becomes problematic.

libc++ is choosen here as a workaround as its standard library does not contain such deleted constructor.

### Steam Runtime (experimental)

Steam Runtime SDK is distributed in the form of OCI images. [SteamRT 3.0 (sniper)](https://gitlab.steamos.cloud/steamrt/sniper/sdk)
is tested. I'm using [docker](https://www.docker.com/) to do the build. Any other Linux container software (eg. Podman)
should work, but I haven't tested.

Any OS that docker runs on should be able to perform the build, as it performs in the containerized environment.

Also GCC 10.3.0 shipped with Steam Runtime SDK does not compile The-Forge 1.56 as is. As a workaround, Clang shipped with the SDK can be used. The libc++ that comes with the sdk seems to be broken so we will use the libstdc++, which is the default runtime library anyway. The version of Libstdc++ comes with the sdk seems to play nicely with The-Forge.

## Building

### Windows

On Windows, just following the common CMake workflow. 

### Linux
On Linux, an extra step is required as one of the executable included are not atrributed correctly. We have to
make the `glslangValidator` becomes executable by chmod it.

```sh
chmod +x "./The-Forge/Common_3/Graphics/ThirdParty/OpenSource/VulkanSDK/bin/Linux/glslangValidator"
```

Also, specify the `libc++` as the C++ standard library. This can be done by specify `toolchain/linux.cmake` as
the toolchain file when configuring the build directory.

```sh
cmake <project-directory> -DCMAKE_TOOLCHAIN_FILE=<project-directory>/toolchain/linux.cmake    
```

And lastly, to build the project.

```sh
make
```

### Steam Runtime (experimental)

Firstly, run `bash` shell inside the sniper container. I'm using `docker run` command here as I want to simply run the command once.
You could also tried create a container and run `bash` inside there as well.

```sh
docker run -it --rm --mount type=bind,source=$(pwd),target=/app registry.gitlab.steamos.cloud/steamrt/sniper/sdk bash
```
This will bind the project directory with the directory `/app` inside the container.

Now, proceed as usual `cmake` project. Specify `clang` as the compiler. This can be done by specify `toolchain/steamrt.cmake` as
the toolchain file when configuring the build directory.

```sh
cmake <project-directory> -DCMAKE_TOOLCHAIN_FILE=<project-directory>/toolchain/steamrt.cmake    
```

And lastly, to build the project.

```sh
make
```

After building the project. You can quit the container environemnt by `exit` the shell. Then just before runing the probject, make sure to `chown` the file to your user (otherwise the application would not run.).

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