import os
import os.path
import platform
import shlex
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path
from textwrap import dedent

from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import Command, setup

FALSE_OPTS = ("", "0", "no", "n", "false", "off", "ignore")


PROJECT = "pjsua2"


class StubGenCommand(Command):
    user_options = []  # type: ignore[var-annotated]

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    @staticmethod
    def _run(build_lib=None, inplace=True):
        with tempfile.TemporaryDirectory() as tmp_dir:
            py_script = (
                dedent("""
                    import sys
                    import mypy.stubgen
                    sys.path.insert(0, '{}')
                    mypy.stubgen.main(['-v', '-p', '{}', '-o', '{}'])
                """)
                .format(Path(build_lib).as_posix() if build_lib else ".", PROJECT, Path(tmp_dir).as_posix())
                .strip()
            )
            args = (sys.executable, "-c", py_script)
            subprocess.run(args, check=True, shell=False)
            src = os.path.join(tmp_dir, f"{PROJECT}.pyi")
            if inplace or not build_lib:
                dst = os.path.join("python", f"{PROJECT}-stubs", "__init__.pyi")
            else:
                dst = os.path.join(build_lib, f"{PROJECT}-stubs", "__init__.pyi")
            print(f"copying {src} -> {dst}")
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            shutil.copy(src, dst)

    def run(self):
        self._run()


class CustomBuildExt(build_ext):
    # override
    def build_extensions(self):
        super().build_extensions()
        self.after_build()

    # override
    def run(self):
        self._run_inplace = self.inplace
        super().run()

    def after_build(self):
        StubGenCommand._run(self.build_lib, self._run_inplace)


def setup_extensions():
    sources = ["src/py_pjsua2.cc"]
    define_macros = []
    extra_compile_args = []
    include_dirs = []
    libraries = []
    library_dirs = []

    if os.name == "nt":  # Windows
        PJ_REBUILD, PJ_DYNAMIC, PJ_DEBUG = map(
            lambda opt: opt.strip().lower() not in FALSE_OPTS,
            (os.environ.get(env_name, "") for env_name in ("PJ_REBUILD", "PJ_DYNAMIC", "PJ_DEBUG")),
        )

        try:
            PJ_DIR = os.environ["PJ_DIR"]
        except KeyError:
            print(
                dedent(
                    """
                    Environment variable `PJ_DIR` SHOULD be set to the directory of ‘pjproject’ repo.

                    On PowerShell, you shall set the environment variable:

                        $Env:PJ_DIR="directory\\of\\pjproject"

                    On CommandPrompt, you shall set the environment variable:

                        set PJ_DIR="directory\\of\\pjproject"
                    """
                ).strip()
                + os.linesep,
                file=sys.stderr,
            )
            exit(-1)

        pj_vs_build_type = "Debug" if PJ_DEBUG else "Release"
        pj_vs_link_type = "Dynamic" if PJ_DYNAMIC else "Static"
        pj_vs_configuration = f"{pj_vs_build_type}-{pj_vs_link_type}"

        define_macros.append(("PJWIN32", "1"))

        if pj_vs_link_type == "Static":
            vc_rtl_flag = "/MT"
        elif pj_vs_link_type == "Dynamic":
            vc_rtl_flag = "/MD"
        else:
            raise RuntimeError(str(pj_vs_link_type))
        if pj_vs_build_type == "Debug":
            vc_rtl_flag += "d"
        extra_compile_args.append(vc_rtl_flag)

        include_dirs.extend(
            [
                f"{PJ_DIR}/pjlib/include",
                f"{PJ_DIR}/pjlib-util/include",
                f"{PJ_DIR}/pjmedia/include",
                f"{PJ_DIR}/pjsip/include",
                f"{PJ_DIR}/pjnath/include",
            ]
        )
        library_dirs.append(f"{PJ_DIR}/lib")
        libraries.extend(["wsock32", "ws2_32", "ole32", "dsound"])
        if platform.machine().lower() == "amd64":
            pj_vs_cpu = "x86_64"
            pj_vs_platform = "x64"
        elif platform.machine().lower() == "x86":
            pj_vs_cpu = "x86"
            pj_vs_platform = "Win32"
        else:
            raise NotImplementedError(platform.machine())
        libraries.append(f"libpjproject-{pj_vs_cpu}-{pj_vs_platform}-vc14-{pj_vs_build_type}-{pj_vs_link_type}")

        # 构建 pjproject
        if PJ_REBUILD:
            # eg: msbuild pjproject-vs14.sln -target:pjsua -m -property:Configuration=Release-Static -property:Platform=x64
            try:
                subprocess.run(["msbuild", "-version"], cwd=PJ_DIR, check=True, shell=False)
            except FileNotFoundError as err:
                print(
                    dedent(
                        f"""
                        Failed to execute ‘MsBuild’: {err}

                        Make sure to run the command on “Developer PowerShell for VS” or “Developer Command Prompt for VS” !
                        """
                    ).strip()
                    + os.linesep,
                    file=sys.stderr,
                )
                exit(-2)
            args = [
                "msbuild",
                "-m",
                "pjproject-vs14.sln",
                "-target:pjsua",
                "-m",
                f"-property:Configuration={pj_vs_configuration}",
                f"-property:Platform={pj_vs_platform}",
            ]
            subprocess.run(args, cwd=PJ_DIR, check=True, shell=False)
    else:
        _, output = run_command_then_get_output("pkg-config --cflags libpjproject")
        extra_compile_args.extend(shlex.split(output))
        _, output = run_command_then_get_output("pkg-config --libs-only-L libpjproject")
        library_dirs.extend(extract_long_options(output))
        _, output = run_command_then_get_output("pkg-config --libs-only-l libpjproject")
        libraries.extend(extract_long_options(output))

    return [
        Pybind11Extension(
            PROJECT,
            sources,
            define_macros=define_macros,
            extra_compile_args=extra_compile_args,
            include_dirs=include_dirs,
            library_dirs=library_dirs,
            libraries=libraries,
        )
    ]


def run_command_then_get_output(command, raise_error=True):
    print(command)
    p = subprocess.run(shlex.split(command), capture_output=True)
    if raise_error:
        p.check_returncode()
    else:
        if p.returncode:
            if p.stderr:
                s = p.stderr.decode().strip()
                print(s, file=sys.stderr)
                return p.returncode, s
            else:
                return ""
    if p.stdout:
        s = p.stdout.decode().strip()
        print(s)
        return p.returncode, s
    else:
        return p.returncode, ""


def extract_long_options(s):
    return [x[2:] for x in shlex.split(s) if len(x) > 2 and x.startswith("-")]


setup(
    ext_modules=setup_extensions(),
    # Currently, build_ext only provides an optional "highest supported C++ level" feature,
    # but in the future it may provide more features.
    cmdclass={
        "build_ext": CustomBuildExt,
        "stubgen": StubGenCommand,
    },
)
