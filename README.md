Video Processor
===============

***Video Processor: High-end video processing on live data for the rest of us.***

VideoProcessor is a Windows live-video player; it couples a capture card to to a DirectShow renderer and takes care of all the plumbing in such a way the metadata stays intact. This allows advanced renderers to do things like 3d LUT, HDR tone mapping, scaling, deinterlacing and much more which can significantly improve image quality on the majority of displays. It is especially useful for accurate color-correction and HDR-like display on low lumen devices like projectors.

**HDCP**

There is one snag through, all high quality copyright protected video is protected with High-bandwidth Digital Content Protection (HDCP). No retail capture card is allowed to forward unprotected video data to it's clients. Therefore, if you connect a HDCP source to a capture card the video output will be disabled or blank. VideoProcessor is just a client of your capture card and hence if your capture card does not output video because of HDCP, there is no video to process. VideoProcessor cannot strip, circumvent or work around HDCP in any way, shape or form, it can only process what it is given.

Devices which can remove this protection are available, allowing for HDCP protected sources being captured by your capture card, but their legality depends on your jurisdiction and use. Ensuring compliance with your local laws, and feeding your capture card data it can forward to VideoProcessor, is your responsibility. 

**madVR**

The most well known, and in my opinion best, renderer is madVR. Though before using it you must understand the legal situation. madVR has two paths, a limited-free-to-use DirectShow library which MadShi releases on his [website](http://madvr.com/) and a commercial appliance called [Envy](https://madvrenvy.com/) which is a video processor software hardware combination. Because of the conflict of interest this causes, madVR has adjusted the license terms of the limited-free-to-use to be illegal for use in video capture applications such as VideoProcessor. madVR has updated all downloadable releases with the new license that prohibits its use in VideoProcessor. 
You can find an original old release (09217), which has the previous license, mirrored [here](http://www.dennisfleurbaaij.com/temp/madVR%2009217%20-%20old%20license.zip). madVR is not included in the VideoProcessor download, you are responsible for downloading and installing an appropriate version.

**Showtime!**

With all that out of the way, get grab the latest release from https://github.com/defl/videoprocessor and enjoy :)


# Community and bleeding edge

Note that VideoProcessor is under very heavy development and that there will be many unpublished experimental releases. You can check out the [AVSForum VideoProcessor thread](https://www.avsforum.com/threads/videoprocessor.3206050/) for these and for support.


# Installing it

- Build a madVR capable computer
  - [AVSForum: Guide: Building a 4k HTPC for madVR](https://www.avsforum.com/threads/guide-building-a-4k-htpc-for-madvr.2364113/)
  - Add capture card, see below
- Install VS2019 x64 runtime
- Install capture card (see below) and drivers, verify the card and capture works by running the vendor's capture application.
- Download VideoProcessor.exe 
- Configure madVR
  - [KODI: Set up madVR](https://forum.kodi.tv/showthread.php?tid=259188) - very complete guide besides newer things like tonemapping
  - [ronaldverlaan.nl/download/htpc.pdf](http://www.ronaldverlaan.nl/download/htpc.pdf) - recent and very complete but in Dutch
- Enjoy
- (After this there will be lots of madVR fiddling and probably ordering a bigger GPU because you've figured out that the highest settings are just a bit better. Welcome to the hobby.)



# Capture cards

VideoProcessor has a wide range of applications. You can do as little as just some color correction on low end inputs like NTSC all the way up to HDR tone mapping a 4k HDR source. As such there is a very wide variety of cards it can potentially work with. The following list provides cards which provide HDMI 2.0 or up, 4K or up, HDR or better and >=10bits - which covers the most common use case of processing a HDR stream and polishing + tone mapping it to your display.



**Tested and confirmed working**

If you can spare it, the BlackMagic DeckLink Quad HDMI can do 4k60, which makes your life easier as many GUIs and menus are in this format. The Mini Recorder 4K is significantly cheaper but can cause some confusion to your systems as it is max 30fps at 4K, this isn't perfectly recognized by all systems which potentially will require fiddling to get it working. For >99.9% of the movies out there the best (value) choice is as good as the high-end choice.

 * [BlackMagic DeckLink Mini Recorder 4k](https://www.blackmagicdesign.com/nl/products/decklink/techspecs/W-DLK-33) (4k30) (€~200) <-- best (value) choice
 * [BlackMagic DeckLink Quad HDMI Recorder](https://www.blackmagicdesign.com/products/decklink/techspecs/W-DLK-36) (4k60) (€~500) <-- high-end choice



**Might work, but untested**

- [DeckLink 4K Extreme 12G](https://www.blackmagicdesign.com/nl/products/decklink/techspecs/W-DLK-25) (4k60) (€~750)



**Won't work** **(*yet*)**

The following cards have capable hardware but are not supported; getting them work is most likely possible but might involve high procurement and/or engineering costs. The rules for getting them added are simple: it needs a published API (either vendor-provided or reverse engineered) and I need the hardware or you send a pull-request.

- [AVerMedia CL511HN](https://www.avermedia.com/professional/product/cl511hn/overview) (4k60, pass-through) (€~900) - There is an SDK available which looks usable
- [Magewell Pro Capture HDMI 4K Plus](https://www.magewell.com/products/pro-capture-hdmi-4k-plus) (4k60) (€~1000) - There is an SDK available
- [Magewell Pro Capture HDMI 4K Plus LT](https://www.magewell.com/products/pro-capture-hdmi-4k-plus-lt) (4k60, pass-through) (€~1100) - There is an SDK available
- [AVerMedia Live Gamer 4K (GC573)](https://www.avermedia.com/us/product-detail/GC573) (4k60, pass-through) (€~200) - No publicly available API/SDK
- [Elgato 4K60 pro](https://www.elgato.com/en/game-capture-4k60-pro) (4k60, pass-through) (€~250) - No publicly available API/SDK



# System requirements

VideoProcessor itself takes very little CPU. The capture card drivers often only take a decent amount of memory (gigs) but little CPU, the rest is madVR. MadVR can be a massive resource drain; at maximum settings when working on a 4K high frame rate feed there simply is no available hardware which can sustain it (RTX3090 included). You'll need an AVX capable CPU (which is anything younger than a decade).

Luckily if you tone it down a bit it works well with quite modest hardware. There are quite a few guides on this, as linked above, so a bit of research will get you a long way. Do note that you will need a proper GPU if you want to do anything with 4k input, output or image enhancement. There have been reports of significant frame drops handling 4K on recent Intel GPUs, while 1080p was ok without image enhancements. Generally Nvidia is strongly preferred.

For reference, I'm developing/using it on an Intel 11400 + 16GB ram + Nvidia GTX 1660 + BlackMagic DeckLink Mini Recorder 4k which is enough for my purposes which is 4K HDR input, 1080p SDR output, 3d LUT plus some minor enhancements.


# FAQ

**Renderer shows black screen - with valid input**

- Are you sure you're capturing something which is outside of what the card can pass along? For example 4k>30 with Blackmagic Recorder 4K mini will lead to this.
- Did you set the correct output display modes in madVR? (2160p23, 2160p24 etc)?

**I have frame drops, jitter, choppy image or similar performance problems**

- You are using an Nvidia card right? Intel GPUs will not cut it and lead to all sorts of drops.
- Ensure your capture card is not sharing it's PCIe bandwidth with something else. Specifically your graphics card.
- Ensure that your card is getting it's full PCIe bandwidth. The BlackMagic cards will show their bandwidth in the Capture Device -> Other properties which has to be link >=2 and width >=4.
- Ensure you didn't set madVR to be too resource intensive. look at it's "max stats (5s)" in the on-screen debug and ensure that's not beyond your frame rate
- Do not run other high (memory) bandwidth applications at the same time. 4k30 12 bit is pushing over 13gbps and that data needs to be in RAM and processed by your CPU several times, which can load up your memory bus quite a bit
- Full screen is generally smooter than windowed
- Use the queue.
- Press the reset button after starting the video.
- If your capture latency (in the gui) is high (anything over 20ms) then issues will appear. Ensure you have no drops/misses. Ideally your clock lead is very slightly positive, set it by changing the frame offset.

**Can this display >=10bit?**

- Yes, >=10 bit capture input, >=10 bit transfer to madVR, 10 bit D3D11 into >=10 bit screen have all been observed working
- Example: Use a PS4Pro, set up 12 bit RGB mode and hook the output of your video card to a computer monitor. Once you fullscreen you should be seeing 10bit and madVR will tell you so in it's debug output.


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

# Commercial alternatives

The people behind madVR also have a commercial offering called [MadVR Envy](https://madvrenvy.com/). It does what this program does but is a complete device, offers support, will be better, has more magic and it's HDCP certified meaning it can show HDCP protected content out of the box AND there is no DIY-ing involved. 

*If you have the means I would recommend you buy their product and support madVR development*

**Partial solutions**

madVR is unique in it's abilities and quality, nothing else comes close. For those of you who don't want everything and are on a budget (relatively speaking) the following might be of interest:

- Lumagen makes a device called the [Radiance Pro](http://www.lumagen.com/testindex.php?module=radiancepro_details) which can also do HDR tonemapping.
- Higher end JVC projectors can do HDR tonemapping internally.
- Higher end projectors and OLED TVs often can do 3DLUT internally.
- There are a lot of lower-end devices which can do 3dLUT, [DisplayCalibrations has a good overview of them](https://displaycalibrations.com/lut_boxes_comparisons.html).

# License & legal

This application is released under the GNU GPL 3.0, see LICENSE.txt. 

Parts of this code are made and owned by others, for example SDKs and the madVR interface; in all such cases there are LICENSE.txt and README.txt files present to point to sources, attributions and licenses.

I'm not affiliated or connected with any of the firms mentioned above.


------

 Copyright 2021 [Dennis Fleurbaaij](mailto:mail@dennisfleurbaaij.com)
