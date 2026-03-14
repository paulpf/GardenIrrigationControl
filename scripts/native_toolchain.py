Import("env")

import os

# Use a locally installed PlatformIO MinGW toolchain for native builds.
pio_home = os.path.expanduser("~/.platformio")
bin_dir = os.path.join(pio_home, "packages", "toolchain-gccmingw32", "bin")

if os.path.isdir(bin_dir):
    env.PrependENVPath("PATH", bin_dir)
    env.Replace(
        CC="gcc",
        CXX="g++",
        AR="ar",
        RANLIB="ranlib",
        GDB="gdb",
    )
    # Force static MinGW runtime linkage to avoid external DLL execution blocks.
    env.Append(
        LINKFLAGS=[
            "-static",
            "-static-libgcc",
            "-static-libstdc++",
        ]
    )
else:
    print("ERROR: Missing MinGW toolchain at: {}".format(bin_dir))
    print("Install with: platformio pkg install -g -t platformio/toolchain-gccmingw32")
    env.Exit(1)
