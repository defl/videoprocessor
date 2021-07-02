![VideoProcessor banner](https://github.com/defl/videoprocessor/blob/main/images/vp%20banner.png)

***:film_projector: High-end video processing on live data for the rest of us.***

VideoProcessor is a live-video player for Windows; it couples a capture card to to a DirectShow renderer and takes care of all the plumbing in such a way the metadata stays intact. This allows advanced renderers to do things like 3d LUT, HDR tone mapping, scaling, deinterlacing and much more which can significantly improve image quality on the majority of displays. It is especially useful for accurate color-correction and HDR-like display on low lumen devices like projectors.

**HDCP**

There is one snag through, all high quality copyright protected video is protected with High-bandwidth Digital Content Protection (HDCP). No retail capture card is allowed to forward unprotected video data to it's clients. Therefore, if you connect a HDCP source to a capture card the video output will be disabled or blank. VideoProcessor is just a client of your capture card and hence if your capture card does not output video because of HDCP, there is no video to process. VideoProcessor cannot strip, circumvent or work around HDCP in any way, shape or form, it can only process what it is given.

Devices which can remove this protection exist, but their legality depends on your jurisdiction and use. Ensuring compliance with your local laws, and feeding your capture card data it can forward to VideoProcessor, is your responsibility.

# Community and bleeding edge

Note that VideoProcessor is under very heavy development and that there will be many unpublished experimental releases. You can check out the [AVSForum VideoProcessor thread](https://www.avsforum.com/threads/videoprocessor.3206050/) for the state of the art and for support.


# Installing it

- Build a computer capable of rendering + add capture card, see below
- Install VS2019 x64 runtime
- Install capture card (see below) and drivers, verify the card and capture works by running the vendor's capture application.
- Download VideoProcessor.exe 
- Enjoy :popcorn:

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

VideoProcessor itself takes very little CPU. The capture card drivers often only take a decent amount of memory (gigs) but little CPU, the rest is the renderer. Some renderers  can be a massive resource drain; at maximum settings when working on a 4K high frame rate feed there simply is no available hardware which can sustain them (RTX3090 included). You'll need an AVX capable CPU (which is anything younger than a decade).

Luckily if you tone it down a bit it works well with quite modest hardware. There are quite a few guides on tuning your system and renderer of choice, so a bit of research will get you a long way. Do note that you will need a proper GPU if you want to do anything with 4k input, output or image enhancement. There have been reports of significant frame drops handling 4K on recent Intel GPUs, while 1080p was ok without image enhancements. Generally Nvidia is strongly preferred.

For reference, I'm developing/using it on an Intel 11400 + 16GB ram + Nvidia GTX 1660 + BlackMagic DeckLink Mini Recorder 4k which is enough for my purposes which is 4K HDR input, 1080p output, 3D LUT for color correction and some minor enhancements.


# FAQ

**Renderer shows black screen - with valid input**

- Are you sure you're capturing something which is outside of what the card can pass along? For example 4k>30 with Blackmagic Recorder 4K mini will lead to this.
- Did you set the correct output display modes in your renderer? Some renderers do refresh rate switching only correctly if you configure it.
- HDCP protected stream will also have no output

**I have frame drops, jitter, choppy image or similar performance problems**

- Ensure your capture card is not sharing it's PCIe bandwidth with something else. Specifically your graphics card.
- Ensure that your card is getting it's full PCIe bandwidth. The BlackMagic cards will show their bandwidth in the Capture Device -> Other properties which has to be link >=2 and width >=4.
- Ensure you didn't set your renderer to be too resource intensive. Anything close-to the 1/framerate render time is probably a bad idea. At 24fps you should stay in the low 30ms.
- Do not run other high (memory) bandwidth applications at the same time. 4k30 12 bit is pushing over 13gbps and that data needs to be in RAM and processed by your CPU several times, which can load up your memory bus quite a bit.
- Full screen is generally smooter than windowed.
- Use the queue. Don't let it fill up. You can press the reset button to clear it or leave it in auto.
- If your capture latency (in the gui) is high (anything over 20ms) then issues will appear. Ensure you have no drops/misses. Ideally your DS renderer latency is very slightly negative (= the frame you just gave your renderer needs to be rendered in the very near future, rather than it being late if this number is positive), set it by changing the timing clock frame offset or just using the auto mode there.
- For advanced renderers: You are using an Nvidia card right? Intel GPUs will not cut it and lead to all sorts of drops.

**Can this capture, process and display >=10bit?**

- Yes. NVidia 12bit output, D3D11 10bit rendering window and 12bit input have been observed working.

# Screenshot

![VideoProcessor banner](https://github.com/defl/videoprocessor/blob/main/images/screenshot.png)

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

VideoProcessor is a DIY project to cheaply get something basic going. And while that's fun, it is very far from a well supported professional application which has non-breaking updates, proper support and rock solid performance. It-just-works is highly underrated in home cinemas and scores major points on the WAF scale in my experience. So in case you're looking for something more pro:

- madVR labs makes a device called the [Envy](https://madvrenvy.com/) which can do both HDR tonemapping and 3d luts
- Lumagen makes a device called the [Radiance Pro](http://www.lumagen.com/testindex.php?module=radiancepro_details) which can also do HDR tonemapping.
- Higher end JVC projectors can do HDR tonemapping internally.
- Higher end projectors and OLED TVs often can do 3DLUT internally.
- There are a lot of lower-end devices which can do 3dLUT, [DisplayCalibrations has a good overview of them](https://displaycalibrations.com/lut_boxes_comparisons.html).

# License & legal

This application is released under the GNU GPL 3.0 for non-commercial usage, commercial usage to build and sell video processor systems is not allowed, see LICENSE.txt. 

Parts of this code are made and owned by others, for example SDKs; in all such cases there are LICENSE.txt and README.txt files present to point to sources, attributions and licenses.

I'm not affiliated or connected with any of the firms mentioned above.

------

 Copyright 2021 [Dennis Fleurbaaij](mailto:mail@dennisfleurbaaij.com)
