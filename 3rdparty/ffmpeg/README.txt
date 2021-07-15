This is a binary build from ffmpeg

License and attribution
=======================
Built from ffmpeg fork at https://github.com/defl/FFmpeg
    This has a r12b decoder plus support for non-threaded v210 decoding, both
    which are required. (I tried for a month or so to get the changes into ffmpeg
    but lost interest given the process, method of picking and lack of interest.)
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
          * pacman -S rsync
          * pacman -S mingw-w64-x86_64-gcc
          * pacman -S pkg-config

* Open a VS2019 x64 shell
  * C:\msys64\msys2_shell.cmd

    * cd /c/Users/User/workspace/
    * mkdir -p ffmpeg_build_release
    * cd ffmpeg_build_release
    * ../ffmpeg/configure --toolchain=msvc --disable-shared --enable-static --arch=x86_64 --target-os=win64 --enable-asm --enable-x86asm --disable-avdevice --disable-doc --disable-bzlib --disable-libopenjpeg --disable-iconv --disable-zlib --disable-libopus --disable-encoder=libopus --disable-decoder=libopus --disable-encoder=opus --disable-decoder=opus --disable-mediafoundation --enable-gpl --disable-network --prefix=../ffmpeg_build_release --extra-cflags="-MD" --extra-cxxflags="-MD"
    * make -j6
    * make install

    * cd /c/Users/User/workspace/
    * mkdir -p ffmpeg_build_debug
    * cd ffmpeg_build_debug
    * ../ffmpeg/configure --toolchain=msvc --disable-shared --enable-static --arch=x86_64 --target-os=win64 --enable-asm --enable-x86asm --disable-avdevice --disable-doc --disable-bzlib --disable-libopenjpeg --disable-iconv --disable-zlib --disable-libopus --disable-encoder=libopus --disable-decoder=libopus --disable-encoder=opus --disable-decoder=opus --disable-mediafoundation --enable-gpl --disable-network --prefix=../ffmpeg_build_debug  --extra-cflags="-MDd" --extra-cxxflags="-MDd" --enable-debug
    * make -j6
    * make install

    * cd /c/Users/User/workspace/
    * mkdir -p ffmpeg_build_fate
    * cd ffmpeg_build_fate
    * ../ffmpeg/configure --toolchain=msvc --arch=x86_64 --target-os=win64 --enable-asm --enable-x86asm  --disable-bzlib --disable-libopenjpeg --disable-iconv --disable-zlib --disable-libopus --disable-encoder=libopus --disable-decoder=libopus --disable-encoder=opus --disable-decoder=opus
    * make -j6 fate

  * C:\msys64\mingw64.cmd
    * cd /c/Users/User/workspace/
    * mkdir -p ffmpeg_build_gcc
    * cd ffmpeg_build_gcc
    * ../ffmpeg/configure --arch=x86_64 --enable-asm --enable-x86asm  --disable-bzlib --disable-libopenjpeg --disable-iconv --disable-zlib --disable-libopus --disable-encoder=libopus --disable-decoder=libopus --disable-encoder=opus --disable-decoder=opus
    * make -j6 fate


* Install libs is just a manual copy 
  * (Note only libavutil and libswscale were used and installed)
  * in ffmpeg_build_debug (for example):
    * Copy all relevant files from lib\* to the right lib dir in 3rdparty\ffmpeg\lib\
    * Copy all relevant files from include\* to 3rdparty\ffmpeg\include\
