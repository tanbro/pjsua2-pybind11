# PJSUA2 PyBind11

softphone 客户端软件使用 [pjproject][] 作为 SIP UAC 的功能实现基础库。
尽管 [pjproject][] 声称它的高层 API 接口直接支持 C++, Java, C#, Python 等（通过 [SWIG][]），但实际试用时我们发现其 [SWIG][] 脚本十分老旧缺乏维护，且在 Windows 下有肉眼可见的问题 —— 某些情况下导致程序崩溃。

为了保证软件质量，我们决定“手搓”一个 [pjproject][] 高层 API 的 Python language wrapper。
考虑到 [pjproject][] 的高层 API 是基于 C++ 的面向对象风格的较完备接口，我们采用 [pybind11][] 进行 C++ 到 Python 的包裹。
严格来讲这不算是“从零手搓”，而是“半自动”的。但也足够我们仔细的进行内存管理，避免危险。

我们将 [pjproject][] 的 [pybind11][] Wrapper 独立出来，形成了这个单独的子项目。

## 准备工作

> 📚 **参考**: \
> <https://docs.pjsip.org/en/latest/get-started/posix/build_instructions.html>

- Windows

    此小节仅记录在 x86_64 / Windows 10 64bit 上的步骤。

    1. 下载安装 git for windows，需要 git 2.0 或以上版本

    1. 下载安装 VisualStudio 2017 或以上（实际开发中使用的是 VisualStudio 2022）, 安装时要选择 “使用 C++ 的桌面开发”

    1. 安装 Python for Windows

       - 如果不需要调试 C++ 部分代码，而是仅需要生成动态库文件，可以直接通过 Microsoft Store 安装 Python。

         > 🔖 **Tip**:\
         > 进行此种安装之后，可以在 Windows 的 “系统设置” ➡️ “添加或删除程序” ➡️ “应用执行别名” 设置 `python`, `pip` 等全局命令所对应的执行文件。

       - 如果需要调试 C++ 部分代码，有这样几种安装方式:

         - 访问 <https://www.python.org/downloads/windows/> 下载它的 Windows 安装包，然后安装

         - 使用 [winget][] 安装，例如:

           ```powershell
           winget install --interactive "Python 3.12"
           ```

         > ❗ **Important**: \
         > 在安装界面中，选择“自定义”安装，并在选项中勾选上:
         > - ☑️ pip
         > - ☑️ py launcher
         > - ☑️ Download debugging symbols
         > - ☑️ Download debug binaries
         > 如果不安装调试文件，将与 Microsoft Store 安装无异。所以使用 [winget][]安装时，注意 `--interactive` 命令行参数，以便进入安装程序的图形界面勾选上述选项。

         ------

         > 🔖 **Tip**:
         >
         > - 如果安装时勾选了“py launcher”，则 Python 的启动命令是 `py`，而不是 `python`。
         > - 如果勾选了（默认未选中） “Add Python to environment variables”，则 Python 的启动命令是`python`。但这种选择方式可能造成不同版本的冲突，不推荐。
         > - 如果两者都没有勾选，则没有全局 Python 启动命令，需要手动设置 `PATH` 环境变量解决这个问题。

       > ℹ️ **Note**: \
       > 这个项目的软件包设置的 Python 版本要求是 `requires-python = ">=3.8"`，但实际开发过程中一直是采用 Python 12，建议使用 Python 3.10 以上的版本。

    1. (*可选*)下载安装 [CMake][] (可以通过 [winget][] 安装)

       [CMake][] 主要用于配合 IDE 提供代码提示和图形化的项目管理。如果不需要配合 IDE 使用，可以不予理会。

- Ubuntu 24.04 LTS

  使用 apt 安装:

  ```bash
  sudo apt install libasound2-dev build-essential cmake pkg-config python3 python3-dev python3-setuptools python3-pip python3-venv python3-pybind11
  ```

  其中, `libasound2-dev` 用于提供访问音频设备的 ALSA 驱动支持。

## 构建 PJPROJECT

此小节仅记录在 x86_64 / Windows 10 64bit 上的步骤。

下载 [pjproject][] 最新的稳定版（本项目目前使用的是 2.14.1，强烈建议保持一致）。
记录其目录位置，设置到环境变量 **`PJ_DIR`** 。

参考 <https://docs.pjsip.org/en/latest/get-started/windows/build_instructions.html> ：

1. 在目录 `pjlib/include/pj` 新建头文件 `config_site.h`，这个文件可以为空（默认值），可参考 <https://docs.pjsip.org/en/latest/get-started/guidelines-development.html#config-site-h>；
1. 由于开发环境的 Windows SDK 与平台工具集版本与项目代码指定的版本大概率不一致，应该使用 VisualStudio 打开解决方案，并按照提示升级到最新的 VisualStudio 项目格式，以及重定项目目标到与当前开发环境匹配的 Windows SDK 与平台工具集版本，忽略不支持的项目，然后再构建。当然也可手动修改项目文件的 XML 内容，如果熟悉这个冷知识。
1. (*可选*) 在项目列表中，将 `pjsua` "设为启动项目"；
1. (*可选*) 选择构建类型与目标平台的组合，本项目确定可以支持的有:
   - Release-STATIC | x64
   - Debug-STATIC | x64
   - Win32 与 Dynamic(DLL) 尚未尝试。
   - ARM64 尚未尝试
   为上述构建类型与目标平台的组合生成 `pjsua` 项目。生成的库文件在 `lib` 目录，形如 `libpjproject-x86_64-x64-vc14-Release-Static`；

如果使用 `MSBuild` 进行构建（同样需要使用 VisualStudio 打开以升级解决方案与项目文件），应在 "Developer PowerShell for VS" 或 "Developer Command Prompt for VS" 命令行环境下执行项目构建命令，以 `Debug-STATIC | x64` 为例:

```powershell
MSBuild pjproject-vs14.sln -target:pjsua -m -p:Configuration=Debug-Static -p:Platform=x64
```

## 构建这个子项目

此小节仅记录在 x86_64 / Windows 10 64bit 上的步骤。

开发时，既可以使用 [CMake][] 构建，也可以使用 [setuptools][] 构建。
前者主要用于开发调试，后者主要用于打包发布。

> 🔖 **Tip**:\
> 许多步骤都需要使用 [pip][] 安装 —— 这需要访问互联网！
> 如果 [pip][] 官方仓库速度太慢，可以用阿里云或者腾讯云的免费镜像。
> 设置方法例子:
>
> ```bash
> pip config set --user global.index-url https://mirrors.aliyun.com/pypi/simple
> ```

- 使用 [setuptools][] 构建

  这种方式适合的场景：不修改 C++ 代码，仅生成 Python Native 模块；获取用于打包发布的生成物；少量修改 C++ 代码，做简单的调试；

  > ❗ **Caution**:
  >
  > - 要将 [pjproject][] 目录的路径设置到环境变量 `PJ_DIR`；
  > - 如果打开了 `PJ_REBUILD` 开关，就需要在 `Developer PowerShell for VS` 或 `Developer Command Prompt for VS` 中执行命令；
  > - 这种构建方式直接调用了 [setuptools][] 脚本（`setup.py`），需要预先使用 [pip][] 在当前的 Python 开发环境安装好这个子项目的开发依赖（“pjsua2pybind11/requirements.txt”文件定义），才能正确执行。安装命令见下一步骤；

  1. 使用 [pip][] 在你当前的 Python 开发环境（强烈建议使用一个单独的 [venv][] 作为 Python 开发环境）安装这个子项目构建和打包所需的几个依赖包:

     ```bash
     pip install -r pjsua2pybind11/requirements.txt
     ```

  1. 构建

     这个子项目的 [setuptools][] 脚本提供数种构建方式（**需要在 pjsua2pybind11 目录执行**）：

     - 原地构建（推荐的方式）

       这种方式适合少量修改 C++ 代码后快速构建，以及从代码快速生成开发所需的原生模块。

       ```bash
       cd pjsua2pybind11
       python setup.py build_ext --inplace
       ```

       命令执行后，共享/动态库文件将输出到目录 `pjsua2pybind11/python`。在 Windows 中，文件名形如: `pjsua2.cp312-win_amd64.pyd`。

     - 完整构建原生部分:

       ```bash
       cd pjsua2pybind11
       python setup.py build_ext
       ```

       命令执行后，仅生成共享/动态库（这个项目实际上也只有共享/动态库文件）。
       输出目录形如 `pjsua2pybind11/build/lib.win-amd64-cpython-312`

     - 完整构建:

       ```bash
       cd pjsua2pybind11
       python setup.py build
       ```

       这个项目只有共享/动态库文件，所以执行 `python setup.py build` 与执行 `python setup.py build_ext` 实际上没有区别。

       输出目录形如 `pjsua2pybind11/build/lib.win-amd64-cpython-312`

     > 🔖 **Tip**:
     >
     > - 如果构建这个子项目之前，尚未构建 [pjproject][]，可以设置环境变量 `PJ_REBUILD`
     >
     >   这样，使用 [setuptools][] 脚本时，就会首先构建 [pjproject][]。
     >   注意需要事先用 VisualStudio 打开后升级 [pjproject][] 解决方案与项目文件
     >
     > - 这种情况下，`python setup.py ...` **要在 Developer PowerShell / CommandPrompt for VS 中执行**。
     >
     >   例如：
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

- 使用 [CMake][] 构建

  这种方式适合：配合支持 [CMake][] 的 `C`/`C++` *IDE* 进行开发和调试

  1. 使用 [pip][] 在你当前的 Python 开发环境（强烈建议使用一个单独的 [venv][] 作为 Python 开发环境）安装 [pybind11][] 等开发所需的依赖软件:

     ```bash
     pip install -r pjsua2pybind11/requirements.txt
     ```
  1. 按照本地开发环境的实际情况修改 `CMakePresets.json.in` 为 `CMakePresets.json` 的配置文件，调用 [CMake][] 执行 workflow

     例如, 将 `CMakePresets.json` 修改为如下：

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

     其中的 `"cacheVariables"`变量按照实际开发环境修改，例如:

     - "PJ_DIR": 设置为 [pjproject][] 源码项目的目录

     - "pybind11_DIR": 设置为 [pybind11][] 的 [CMake][] 脚本目录

       在使用 [pip][] 安装好依赖软件后，可以使用以下命令查看 [pybind11][] 的 [CMake][] 脚本目录:

       ```bash
       pybind11-config --cmakedir
       ```

     设置好了之后，执行 [CMake][] 工作流，如:

     ```bash
     cmake --workflow --preset win
     ```

     当然也可分步骤执行，例如:

     ```bash
     # configure:
     cmake --preset win-vs2022

     # build:
     cmake --build --preset win-vs2022-release
     ```

  生成的 Python native module 目标文件将输出到目录 `pjsua2pybind11/python`，文件名形如: `pjsua2.cp312-win_amd64.pyd`

  > ❓ **FAQ**
  >
  > - Q: 我的开发机上有多个 Python 环境，该怎么指定？
  > - A: 这种情况比较复杂，需要自行填写 [CMake][] 变量，可以在 preset 配置文件或者命令行指定。变量 `Python3_ROOT_DIR` 定义了要使用的 Python 3 安装的根目录，例如:
  >
  >   ```bash
  >   cd build
  >   cmake -DPJ_REBUILD=yes -DCMAKE_BUILD_TYPE=Debug -Dpybind11_DIR="$(pybind11-config --cmakedir)" -DPython3_ROOT_DIR="C:\Program Files\Python 3.10" ..
  >   ```

## 以可编辑模式安装到 Python 开发环境

为了在开发 softphone 程序的可执行模块时，正确连接到这个子项目生成的原生动态库，以及获取语法提示，我们以“可编辑”模式安装这个子项目到 Python 的开发环境
（强烈建议使用一个单独的 [venv][] 作为 Python 开发环境）。

安装命令是:

```bash
pip install -e ./pjsua2pybind11/
```

[pip][] 在安装时，会使用 [setuptools][] 脚本“原地”构建，并将目录 `pjsua2pybind11/python` 添加到开发环境的 Site 搜索路径。

原地安装完毕后，如果使用 [CMake][] 重新编译了 `C++` 代码，这个模块的 `Python` 语法提示文件(`*.pyi`)不会自动更新。如果想要更新，可以手动执行：

```powershell
cd pjsua2pybind11
python setup.py stubgen
```

它将生成原生模块所提供接口的定义文件(`.pyi`)，输出到 `pjsua2pybind11/python/pjsua2-stub/__init__.pyi` 。

使用 [setuptools][] 编译，会自动更新 `*.pyi` 文件。

> ℹ️ **Note**:\
> 手动执行生成接口文件的命令**需要**:
>
> - 已经使用 [pip][] 安装了这个子项目的开发依赖（`pjsua2pybind11/requirements.txt` 文件定义）；
> - 已经使用 [pip][] 以“编辑模式”安装这个子项目到开发环境，或者已经使用 `python setup.py build_ext --inplace` 原地构建；

## 打包

我们可以将这个子项目进行打包，制作成 [pip][] 可以直接安装的 [wheel][] 格式文件。
后续的发布工作可能会用到。

命令：

```bash
cd pjsua2pybind11
python setup.py build
python -m build -w
```

文件输出在 `pjsua2pybind11/dist`，文件名形如 `pjsua2-0.1-cp312-cp312-win_amd64.whl`。

如果不需要打包模块的接口定义文件，请不要调用接口定义生成命令；如果已经生成了，可以在打包之前删除这个文件（`pjsua2pybind11/python/pjsua2/__init__.pyi`）。

[pjproject]: https://www.pjsip.org "PJSIP is a free and open source multimedia communication library written in C language implementing standard based protocols such as SIP, SDP, RTP, STUN, TURN, and ICE."
[SWIG]: https://swig.org/ "SWIG is a software development tool that connects programs written in C and C++ with a variety of high-level programming languages."
[pybind11]: https://pybind11.readthedocs.io/ "pybind11 is a lightweight header-only library that exposes C++ types in Python and vice versa, mainly to create Python bindings of existing C++ code."
[CMake]: https://cmake.org/ "CMake is an open source, cross-platform family of tools designed to build, test, and package software."
[winget]: https://learn.microsoft.com/windows/package-manager/winget/ "用户可以在 Windows 10 和 Windows 11 计算机上使用 winget 命令行工具来发现、安装、升级、删除和配置应用程序。 此工具是 Windows 程序包管理器服务的客户端接口。"
[pip]: https://pypi.org/ "pip is the package installer for Python"
[venv]: https://docs.python.org/3/library/venv.html "The venv module supports creating lightweight “virtual environments”, each with their own independent set of Python packages installed in their site directories."
[setuptools]: https://setuptools.pypa.io/ "Setuptools is a fully-featured, actively-maintained, and stable library designed to facilitate packaging Python projects."
[wheel]: https://packaging.python.org/guides/distributing-packages-using-setuptools/#wheels "A wheel is a built package that can be installed without needing to go through the “build” process."
