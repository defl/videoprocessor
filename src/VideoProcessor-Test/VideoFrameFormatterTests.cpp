#include "pch.h"
#include "CppUnitTest.h"

#include <video_frame_formatter/CFFMpegDecoderVideoFrameFormatter.h>


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
	TEST_CLASS(VideoFrameFormatter)
	{
	public:

		TEST_METHOD(CFFMpegDecoderVideoFrameFormatterTest)
		{
			CFFMpegDecoderVideoFrameFormatter vff(
				AV_CODEC_ID_R210,
				AV_PIX_FMT_RGB48LE);

			VideoStateComPtr vs = new VideoState();
			vs->valid = true;
			vs->displayMode = std::make_shared<DisplayMode>(1920, 1080, false /* interlaced */, 24000, 1000);
			vs->videoFrameEncoding = VideoFrameEncoding::V210;  // Actual type not important

			vff.OnVideoState(vs);
			vff.OnVideoState(vs);

			Assert::AreEqual(12441600L, vff.GetOutFrameSize());
		}
	};
}
