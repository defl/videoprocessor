#include "pch.h"
#include "CppUnitTest.h"

#include <video_frame_formatter/CNoopVideoFrameFormatter.h>
#include <video_frame_formatter/CFFMpegDecoderVideoFrameFormatter.h>
#include <video_frame_formatter/CV210toP010VideoFrameFormatter.h>
#include <video_frame_formatter/CV210toP210VideoFrameFormatter.h>


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
	TEST_CLASS(VideoFrameFormatterTests)
	{
	public:

		TEST_METHOD(CNoopVideoFrameFormatterTest)
		{
			CNoopVideoFrameFormatter vff;

			VideoStateComPtr vs = new VideoState();
			vs->valid = true;
			vs->displayMode = std::make_shared<DisplayMode>(1920, 1080, false /* interlaced */, 24000, 1000);
			vs->videoFrameEncoding = VideoFrameEncoding::V210;  // Actual type not important

			vff.OnVideoState(vs);
			vff.OnVideoState(vs);

			Assert::AreEqual(5529600L, vff.GetOutFrameSize());
		}

		TEST_METHOD(CV210toP010VideoFrameFormatterTest)
		{
			CV210toP010VideoFrameFormatter vff;

			VideoStateComPtr vs = new VideoState();
			vs->valid = true;
			vs->displayMode = std::make_shared<DisplayMode>(1920, 1080, false /* interlaced */, 24000, 1000);
			vs->videoFrameEncoding = VideoFrameEncoding::V210;  // Actual type not important

			vff.OnVideoState(vs);
			vff.OnVideoState(vs);

			Assert::AreEqual(6220800L, vff.GetOutFrameSize());
		}

		TEST_METHOD(CV210toP210VideoFrameFormatterTest)
		{
			CV210toP210VideoFrameFormatter vff;

			VideoStateComPtr vs = new VideoState();
			vs->valid = true;
			vs->displayMode = std::make_shared<DisplayMode>(1920, 1080, false /* interlaced */, 24000, 1000);
			vs->videoFrameEncoding = VideoFrameEncoding::V210;  // Actual type not important

			vff.OnVideoState(vs);
			vff.OnVideoState(vs);

			Assert::AreEqual(8294400L, vff.GetOutFrameSize());
		}

		TEST_METHOD(CFFMpegDecoderVideoFrameFormatterR210RGB48LETest)
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

		TEST_METHOD(CFFMpegDecoderVideoFrameFormatterR12BRGB48LETest)
		{
			CFFMpegDecoderVideoFrameFormatter vff(
				AV_CODEC_ID_R12B,
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
