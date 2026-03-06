Import("env")

import os
from os.path import join
from shutil import copy2


toolchain_dir = env.PioPlatform().get_package_dir("toolchain-gccmingw32")
if toolchain_dir:
  toolchain_bin = join(toolchain_dir, "bin")
  os.environ["PATH"] = toolchain_bin + os.pathsep + os.environ.get("PATH", "")
  env["ENV"]["PATH"] = os.environ["PATH"]

  def _copy_runtime_dlls(source, target, env):
    build_dir = env.subst("$BUILD_DIR")
    for dll_name in ("libgcc_s_dw2-1.dll", "libstdc++-6.dll", "libwinpthread-1.dll"):
      copy2(join(toolchain_bin, dll_name), join(build_dir, dll_name))

  env.AddPostAction("$PROGPATH", _copy_runtime_dlls)
