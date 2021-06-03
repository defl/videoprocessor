This is a binary build from ffmpeg

License and attribution
=======================
Built from ffmpeg https://github.com/defl/FFmpeg (at time of writing contains new r12b decoder)
Built with the gpl-2 license, for license and attribution see:
 - MAINTAINERS
 - CREDIT
 - COPYING.GPLv2

Building ffmpeg libs
====================
* Install MSYS2 (msys2-x86_64-20210419)
    * set MSYS2_PATH_TYPE=inherit (either as an environment var or uncomment in C:\msys64\msys2_shell.cmd)
    * Open a VS2019 x64 shell
       * C:\msys64\msys2_shell.cmd
          * which link, if "/usr/bin/link" then "rm /usr/bin/link", needs to be MSVC's
          * pacman -S make
          * pacman -S diffutils
          * pacman -S yasm
          * (no need for pkg-config or gcc or any compiler as we'll use msvc)

* Open a VS2019 x64 shell
  * C:\msys64\msys2_shell.cmd

    * cd /c/Users/User/workspace/ffmpeg/
    * mkdir -p Output/win10_x64_release
    * cd Output/win10_x64_release
    * ../../configure --toolchain=msvc --disable-shared --enable-static --disable-programs --arch=x86_64 --target-os=win64 --enable-asm --enable-x86asm --disable-avdevice --disable-doc --disable-bzlib --disable-libopenjpeg --disable-iconv --disable-zlib --disable-libopus --disable-encoder=libopus --disable-decoder=libopus --disable-encoder=opus --disable-decoder=opus --disable-mediafoundation --enable-gpl --disable-network --prefix=../../Output/win10_x64_release --extra-cflags="-MD" --extra-cxxflags="-MD"
    * make -j6
    * make install

    * cd /c/Users/User/workspace/ffmpeg/
    * mkdir -p Output/win10_x64_debug
    * cd Output/win10_x64_debug
    * ../../configure --toolchain=msvc --disable-shared --enable-static --disable-programs --arch=x86_64 --target-os=win64 --enable-asm --enable-x86asm --disable-avdevice --disable-doc --disable-bzlib --disable-libopenjpeg --disable-iconv --disable-zlib --disable-libopus --disable-encoder=libopus --disable-decoder=libopus --disable-encoder=opus --disable-decoder=opus --disable-mediafoundation --enable-gpl --disable-network --prefix=../../Output/win10_x64_debug  --extra-cflags="-MDd" --extra-cxxflags="-MDd" --enable-debug
    * make -j6
    * make install

* Install libs
  * (Note only libavutil and libswscale were used and installed)
  * Go to build dir (C:\Users\User\workspace\ffmpeg\Output/win10_x64_debug)
  * Copy all relevant files from lib\*
  * Copy all relevant files from include\*
