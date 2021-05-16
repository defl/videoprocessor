Video Processor
===============

***Video Processor: High-end video processing on live data for the rest of us***

Video Processor is a Windows application which couples a capture card to a high quality video renderer ([madVR](http://madvr.com/)). This allows the renderer to do things like 3d LUT, HDR tone mapping, scaling, deinterlacing and much more which can significantly improve visual quality on pretty much any display. It is especially useful for accurate color-correction and HDR-like display on low lumen devices like beamers.

madVR is legendary for it's video processing prowess, but it's not a complete solution. It's "just" a DirectShow renderer and as such it can take input from any DirectShow source. There are sources for pretty much anything, even capture cards. As it stands, only file-based sources (ripped Blue-rays etc) have the HDR metadata which allow madVR to figure out what it's actually displaying. None of the capture card vendors have implemented the required metadata. There is a DirectShow filter which can inject this data ([github.com/defl/directshow_metadata_injector_filter](https://github.com/defl/directshow_metadata_injector_filter)) post-capture but that is not as user-friendly as this application as it requires manual configuration or external hardware and some scripts. VideoProcessor is a one-click solution for this problem allowing live streaming and processing of high end HDR, WCG sources such as a tv boxes, Blu-ray player, DVD player, Apple TV, NVidia Shield etc. 

There is one snag though, for copy protection reasons all interesting video data is protected with High-bandwidth Digital Content Protection (HDCP). No retailed capture card is allowed to forward protected video data. If you connect a HDCP source to a capture card you'll get a black screen or notice that this is not allowed. Cheap devices which can remove this protection are readily available, but it depends on your local jurisdiction if this is allowed and/or enforced.

Get the latest release at http://www.videoprocessor.org/ or the code at https://github.com/defl/videoprocessor.

*Note that madVR also has commercial offering called [MadVR Envy](https://madvrenvy.com/) It does what this program does but is a complete device, offers support, will be better, has more magic and it's HDCP certified meaning no legal gray zone or DIY-ing involved. If you have the means (~$13k) I would recommend you buy their product and support madVR development.* 



# Installing it

- Install capture card (see below) and drivers, verify the card and capture works by running the vendor's capture application.
- Download VideoProcessor.exe 
- Configure madVR
  - TODO: Insert links to guides
- Enjoy
- (After this there will be lots of madVR fiddling and probably ordering a bigger GPU because you've figured out that the highest settings are just a bit better. Welcome to the hobby.)



# Capture cards

VideoProcessor has a wide range of applications. You can do as little as just some color correction on low end inputs like NTSC all the way up to HDR tone mapping a 4k HDR source. As such there is a very wide variety of cards it can potentially work with. The following list provides cards which provide HDMI 2.0 or up, 4K or up, HDR or better and >=10bits - which covers the most common use case of getting a HDR stream and polishing + tone mapping it to your display.

**Tested and confirmed working**

 * [BlackMagic DeckLink Mini Recorder 4k](https://www.blackmagicdesign.com/nl/products/decklink/techspecs/W-DLK-33) (4k30)

**Might work, but untested**

- [DeckLink 4K Extreme 12G](https://www.blackmagicdesign.com/nl/products/decklink/techspecs/W-DLK-25) (4k60)

**Won't work**

The following cards have capable hardware but are not supported; getting them work is most likely possible but might involve high procurement and/or engineering costs. The rules for getting them added are simple: it needs a published API (either vendor-provided or reverse engineered) and I need the hardware or you send a pull-request.

- [AVerMedia CL511HN](https://www.avermedia.com/professional/product/cl511hn/overview) (4k60, pass-through) - There is an SDK available which looks usable
- [AVerMedia Live Gamer 4K (GC573)](https://www.avermedia.com/us/product-detail/GC573) (4k60, pass-through) - No publicly available API/SDK
- [Elgato 4K60 pro](https://www.elgato.com/en/game-capture-4k60-pro) (4k60, pass-through) - No publicly available API/SDK



# System requirements

VideoProcessor itself takes very little CPU. The capture card drivers often just take a decent amount of memory up (gigs) and the rest is madVR. MadVR can be a massive resource drain; at maximum settings when working on a 4K high frame rate feed there simply is no available hardware which can sustain it (RTX3090 included).

Luckily if you tone it down a bit it works well with quite modest hardware. There are tons of threads like [Building a 4K HTPC for madVR](https://www.avsforum.com/threads/guide-building-a-4k-htpc-for-madvr.2364113/) on this so a bit of research will get you a long way.

(For reference, I'm developing/using it on Intel 11400 + 16GB ram + GTX 1660 +[BlackMagic DeckLink Mini Recorder 4k](https://www.blackmagicdesign.com/nl/products/decklink/techspecs/W-DLK-33)  which is enough for my purposes which is 4K HDR input, 1080p tone mapped output.)



# HDCP stripping

In most, but not all, jurisdictions you are not allowed to remove effective encryption and hence HDCP strippers are illegal to use. If you strip HDCP, capture the raw stream and then distribute the material to others you are doing something morally wrong on top of that (even if your local jurisdiction would allow it).

However there are some interesting notes to be placed here:

- A device such is allowed to downgrade HDCP 2+ to HDCP 1.4 ([see legendsky case](https://torrentfreak.com/4k-content-protection-stripper-beats-warner-bros-in-court-1605xx/)). HDCP 1.4 can be considered a broken encoding and in some jurisdictions the removal of a broken encryption is not illegal.
- In some jurisdictions the encryption is at odds with the right for a self-copy of the material.
- So far I've not seen anyone ever convicted of this as the *intent* here is not to pirate, but merely to display from a legal source to a display for private use.

VideoPlayer does not support storage of the data, it can merely push it to madVR. VideoPlayer cannot strip HDCP, you are responsible for providing a HDCP free stream, either because there is no HDCP on the material or because you have removed it by using a device which can do so.

My take is that if anything VideoPlayer will reduce piracy as people are now not required anymore to "rip" Blue-rays so they can be file-played through rendered like madVR. (Ripping is a similar legal gray zone.) With VideoPlayer all can stay on the original disk/streamer and hence less chance that it gets distributed.



# Dev stuff

Get the source from https://github.com/defl/videoprocessor

 * MSVC 2019 community edition

 * Install MFC libraries

 * Debug builds require the [Visual Leak Detector](https://kinddragon.github.io/vld/) Visual C++ plugin

   

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



# License & legal

This application is released under the GNU GPL 3.0, see LICENSE.txt. 

Parts of this code are made and owned by others, for example SDKs and the madVR interface; in all such cases there are LICENSE.txt and README.txt files present to point to sources, attributions and licenses.

I'm not affiliated or connected with any of the firms mentioned above. 



------

 Copyright 2021 [Dennis Fleurbaaij](mailto:mail@dennisfleurbaaij.com)



