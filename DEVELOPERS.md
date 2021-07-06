
# For developers

Get the source from https://github.com/defl/videoprocessor

 * MSVC 2019 community edition
    * Install MFC libraries
    * Debug builds require the [Visual Leak Detector](https://kinddragon.github.io/vld/) Visual C++ plugin
 * You also need to build a special branch of ffmpeg, instructions to do so are in 3rdparty\ffmpeg

**Debugging**

 * Directshow call debugging
    * Open regedit
    * Go to Computer\HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\DirectShow\Debug
    * Make key (=folder) VideoProcessor.exe
    * Set folder rights to user who will run it (or all)
    * Close regedit
    * Start application
    * Open regedit
    * Computer\HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\DirectShow\Debug\VideoProcessor.exe\ should have a bunch of entries like TRACE and LogToFile
    * Set log types to 5 or up
