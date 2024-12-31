# PJSUA2 PyBind11

A Python library wrapped `PJSUA2` of [pjproject][] with [pybind11][].

Despite [pjproject][] claiming its high-level API interfaces directly support C++, Java, C#, Python, etc. (through [SWIG][]), we found its [SWIG][] scripts is not such friendly to Python, So we decided to "hand-roll" a [pjproject][] high-level API Python language wrapper.
Considering [pjproject][]'s high-level API is based on C++ with an object-oriented style and is quite complete, we adopted [pybind11][] for C++ to Python wrapping.
Strictly speaking, this is not a "from-scratch" hand-roll, but rather a "semi-automatic" one. It is sufficient for us to carefully manage memory and avoid dangers.

## Preparation

> 📚 **Reference**: \
> <https://docs.pjsip.org/en/latest/get-started/posix/build_instructions.html>

- Windows

    This section only records the steps on x86_64 / Windows 10 64bit.

    1. Download and install git for windows, version 2.0 or above.

    1. Download and install VisualStudio 2017 or above (actually developed using VisualStudio 2022), and select "Desktop development with C++" during installation.

    1. Install Python for Windows

       - If you do not need to debug C++ code and only need to generate dynamic library files, you can directly install Python through the Microsoft Store.

         > 🔖 **Tip**:\
         > After this installation, you can set the global commands `python`, `pip`, etc., in Windows "Settings" ➡️ "Apps" ➡️ "Apps & features" ➡️ "App execution aliases".

       - If you need to debug C++ code, there are several installation methods:

         - Visit <https://www.python.org/downloads/windows/> to download the Windows installer and install.

         - Use [winget][] to install, for example:

           ```powershell
           winget install --interactive "Python 3.12"
           ```

         > ❗ **Important**: \
         > In the installation interface, select "Custom" installation and check the following options:
         > - ☑️ pip
         > - ☑️ py launcher
         > - ☑️ Download debugging symbols
         > - ☑️ Download debug binaries
         > If debugging files are not installed, it will be the same as installing through the Microsoft Store. So when using [winget][] installation, note the `--interactive` command-line parameter to enter the installer's graphical interface and check the above options.

         ------

         > 🔖 **Tip**:
         >
         > - If "py launcher" is selected during installation, the Python startup command is `py`, not `python`.
         > - If "Add Python to environment variables" is selected (default unchecked), the Python startup command is `python`. But this choice may cause conflicts between different versions, so it is not recommended.
         > - If neither is selected, there is no global Python startup command, and you need to manually set the `PATH` environment variable to solve this problem.

       > ℹ️ **Note**: \
       > The package settings of this project require Python version `requires-python = ">=3.8"`, but the actual development process has always used Python 12. It is recommended to use Python 3.10 or above.

    1. (*Optional*) Download and install [CMake][] (can be installed via [winget][])

       [CMake][] is mainly used in conjunction with IDEs to provide code hints and graphical project management. If not used in conjunction with IDEs, it can be ignored.

- Ubuntu 24.04 LTS

  Install using apt:

  ```bash
  sudo apt install libasound2-dev build-essential cmake pkg-config python3 python3-dev python3-setuptools python3-pip python3-venv python3-pybind11
  ```

  Among them, `libasound2-dev` is used to provide ALSA driver support for accessing audio devices.

## PJPROJECT

This section only records the steps on x86_64 / Windows 10 64bit.

Download the latest stable version of [pjproject][] (this project currently uses 2.14.1, strongly recommended to keep consistent).
Record its directory location and set it to the environment variable **`PJ_DIR`**.

Refer to <https://docs.pjsip.org/en/latest/get-started/windows/build_instructions.html>:

1. In the directory `pjlib/include/pj`, create a new header file `config_site.h`, this file can be empty (default value), refer to <https://docs.pjsip.org/en/latest/get-started/guidelines-development.html#config-site-h>;
1. Due to the development environment's Windows SDK and platform toolset version being inconsistent with the project code specified version, you should use VisualStudio to open the solution and follow the prompts to upgrade to the latest VisualStudio project format, and re-target the project to match the current development environment's Windows SDK and platform toolset version, ignore unsupported projects, and then build. Of course, you can also manually modify the project file's XML content if you are familiar with this cold knowledge.
1. (*Optional*) In the project list, set `pjsua` as the startup project;
1. (*Optional*) Select the build type and target platform combination, this project is confirmed to support:
   - Release-STATIC | x64
   - Debug-STATIC | x64
   - Win32 and Dynamic (DLL) have not been attempted.
   - ARM64 has not been attempted
   Generate the `pjsua` project for the above build type and target platform combination. The generated library files are in the `lib` directory, named like `libpjproject-x86_64-x64-vc14-Release-Static`;

If using `MSBuild` to build (you still need to use VisualStudio to open to upgrade the solution and project files), execute the project build command in the "Developer PowerShell for VS" or "Developer Command Prompt for VS" command line environment, taking `Debug-STATIC | x64` as an example:

```powershell
MSBuild pjproject-vs14.sln -target:pjsua -m -p:Configuration=Debug-Static -p:Platform=x64
```

## Build

This section only documents the steps on x86_64 / Windows 10 64bit.

During development, you can use either [CMake][] or [setuptools][] for building.
The former is mainly used for development and debugging, while the latter is primarily used for packaging and distribution.

> 🔖 **Tip**:\
> Many steps require using [pip][] for installation — this requires internet access!
> If the official [pip][] repository is too slow, you can use free mirrors from Alibaba Cloud or Tencent Cloud.
> Example setup:
>
> ```bash
> pip config set --user global.index-url https://mirrors.aliyun.com/pypi/simple
> ```

- Building with [setuptools][]

  This method is suitable for scenarios where: C++ code is not modified, only Python Native modules are generated; obtaining artifacts for packaging and distribution; minor modifications to C++ code for simple debugging;

  > ❗ **Caution**:
  >
  > - The path to the [pjproject][] directory should be set in the environment variable `PJ_DIR`;
  > - If the `PJ_REBUILD` switch is enabled, commands must be executed in `Developer PowerShell for VS` or `Developer Command Prompt for VS`;
  > - This build method directly calls the [setuptools][] script (`setup.py`), which requires pre-installing the project's development dependencies (defined in "pjsua2pybind11/requirements.txt") using [pip][] in the current Python development environment. Installation command is provided in the next step;

  1. Install the necessary dependencies for building and packaging this sub-project using [pip][] in your current Python development environment (strongly recommended to use a separate [venv][] as the Python development environment):

     ```bash
     pip install -r pjsua2pybind11/requirements.txt
     ```

  2. Build

     The [setuptools][] script of this sub-project provides several build methods (**must be executed in the pjsua2pybind11 directory**):

     - In-place build (recommended method)

       Suitable for quick builds after minor modifications to C++ code and rapid generation of native modules required for development.

       ```bash
       cd pjsua2pybind11
       python setup.py build_ext --inplace
       ```

       After execution, shared/dynamic library files will be output to the `pjsua2pybind11/python` directory. On Windows, filenames resemble: `pjsua2.cp312-win_amd64.pyd`.

     - Complete build of native parts:

       ```bash
       cd pjsua2pybind11
       python setup.py build_ext
       ```

       After execution, only shared/dynamic libraries (this project actually only has shared/dynamic libraries) are generated.
       Output directories look like `pjsua2pybind11/build/lib.win-amd64-cpython-312`

     - Complete build:

       ```bash
       cd pjsua2pybind11
       python setup.py build
       ```

       Since this project only has shared/dynamic libraries, executing `python setup.py build` is essentially the same as executing `python setup.py build_ext`.

       Output directories look like `pjsua2pybind11/build/lib.win-amd64-cpython-312`

     > 🔖 **Tip**:
     >
     > - If [pjproject][] has not been built before building this sub-project, set the environment variable `PJ_REBUILD`
     >
     >   This way, when using the [setuptools][] script, it will first build [pjproject][]. Note that you need to open and upgrade the [pjproject][] solution and project files using VisualStudio beforehand.
     >
     > - In this case, `python setup.py ...` **must be executed in Developer PowerShell / CommandPrompt for VS**.
     >
     >   For example:
     >
     >   ```powershell
     >   **********************************************************************
     >   ** Visual Studio 2022 Developer PowerShell v17.10.1
     >   ** Copyright (c) 2022 Microsoft Corporation
     >   **********************************************************************
     >   PS C:\Program Files\Microsoft Visual Studio\2022\Community> cd C:\Repos\softphone\pjsua2pybind11
     >   PS C:\Repos\softphone\pjsua2pybind11> $Env:PJ_DIR="C:\Repos\pjproject"
     >   PS C:\Repos\softphone\pjsua2pybind11> $Env:PJ_REBUILD="1"
     >   PS C:\Repos\softphone\pjsua2pybind11> cd pjsua2pybind11
     >   PS C:\Repos\softphone\pjsua2pybind11> python setup.py build_ext --inplace
     >   ```

- Building with [CMake][]

  This method is suitable for developing and debugging with an `C`/`C++` *IDE* that supports [CMake][]

  1. Install [pybind11][] and other development dependencies using [pip][] in your current Python development environment (strongly recommended to use a separate [venv][] as the Python development environment):

     ```bash
     pip install -r pjsua2pybind11/requirements.txt
     ```
  2. Modify the configuration file `CMakePresets.json.in` to `CMakePresets.json` according to your local development environment, then call [CMake][] to execute the workflow

     For example, modify `CMakePresets.json` as follows:

     ```javascript
     /// ...
     "configurePresets": [{
          "name": "win-vs2022",
          "inherits": "default",
          "generator": "Visual Studio 17 2022",
          "condition": {
            "type": "equals",
            "lhs": "${hostSystemName}",
            "rhs": "Windows"
          },
          "cacheVariables": {
            "PJ_DIR": "D:\\opensource\\pjproject",
            "pybind11_DIR": "${sourceDir}/../.venv/lib/site-packages/pybind11/share/cmake/pybind11"
          }
     }],
     /// ...
     ```

     Modify the `"cacheVariables"` according to your actual development environment, such as:

     - "PJ_DIR": Set to the directory of the [pjproject][] source code project

     - "pybind11_DIR": Set to the [CMake][] script directory of [pybind11][]

       After installing dependencies using [pip][], you can check the [CMake][] script directory of [pybind11][] with the following command:

       ```bash
       pybind11-config --cmakedir
       ```

     After setting up, execute the [CMake][] workflow, for example:

     ```bash
     cmake --workflow --preset win
     ```

     Or execute step by step, for example:

     ```bash
     # configure:
     cmake --preset win-vs2022

     # build:
     cmake --build --preset win-vs2022-release
     ```

  The generated Python native module target files will be output to the directory `pjsua2pybind11/python`, with filenames resembling: `pjsua2.cp312-win_amd64.pyd`

  > ❓ **FAQ**
  >
  > - Q: How do I specify if I have multiple Python environments on my development machine?
  > - A: This situation is more complex and requires manually filling in [CMake][] variables, which can be specified in the preset configuration file or command line. The variable `Python3_ROOT_DIR` defines the root directory of the Python 3 installation to be used, for example:
  >
  >   ```bash
  >   cd build
  >   cmake -DPJ_REBUILD=yes -DCMAKE_BUILD_TYPE=Debug -Dpybind11_DIR="$(pybind11-config --cmakedir)" -DPython3_ROOT_DIR="C:\Program Files\Python 3.10" ..
  >   ```

[pjproject]: https://www.pjsip.org "PJSIP is a free and open source multimedia communication library written in C language implementing standard based protocols such as SIP, SDP, RTP, STUN, TURN, and ICE."
[SWIG]: https://swig.org/ "SWIG is a software development tool that connects programs written in C and C++ with a variety of high-level programming languages."
[pybind11]: https://pybind11.readthedocs.io/ "pybind11 is a lightweight header-only library that exposes C++ types in Python and vice versa, mainly to create Python bindings of existing C++ code."
[CMake]: https://cmake.org/ "CMake is an open source, cross-platform family of tools designed to build, test, and package software."
[winget]: https://learn.microsoft.com/windows/package-manager/winget/
[pip]: https://pypi.org/ "pip is the package installer for Python"
[venv]: https://docs.python.org/3/library/venv.html "The venv module supports creating lightweight “virtual environments”, each with their own independent set of Python packages installed in their site directories."
[setuptools]: https://setuptools.pypa.io/ "Setuptools is a fully-featured, actively-maintained, and stable library designed to facilitate packaging Python projects."
