#pragma once

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/tracking.hpp>
#include <set>

#include "models.hpp"
#include "DudeSquad.hpp"

int child_plot_process = 0;

void kill_child_processes()
{
	if (child_plot_process > 0) kill(child_plot_process, SIGTERM);
}

struct FrameContext
{
	cv::Mat                         mat;
	dlib::matrix<dlib::rgb_pixel>   matrix;
	std::vector<dlib::rectangle>    faces;
	std::vector<cv::Rect>           cv_faces;
	DudesSquad                      dudesSquad;
};

enum ePriority : uint16_t
{
	PRE_PROCESSING,
	TRACKING,
	HIGHT,
	LOW
};

class IFrameProvider
{
public:
	virtual bool                                   Update() = 0;
	virtual bool                                   IsActive() const = 0;
};

class IFrameHandler
{
public:
	virtual void OnFrame(FrameContext& frame) = 0;
};

class FrameDisplayer : public IFrameHandler
{
public:
	void OnFrame(FrameContext& frame) override
	{
		TRACE_FUNCTION();
		if (!win.is_closed())
		{
			win.clear_overlay();
			win.set_image(frame.matrix);

			// for (auto& face : frame.cv_faces)
			// {
			//     win.add_overlay(dlib::image_window::overlay_rect(openCVRectToDlib(face), dlib::rgb_pixel(0,255,0)));
			// }

			// for (auto& dude : frame.dudesSquad.GetDudes())
			// {
			//     win.add_overlay(dlib::image_window::overlay_rect(dude.GetFacePosition(), dlib::rgb_pixel(0,255,0), dude.GetName().c_str()));
			// }
		}
		else
		{
			kill_child_processes();
			exit(0);
		}

	}
private:
	dlib::image_window win;
};

class FrameCvDisplayer : public IFrameHandler
{
public:
	void OnFrame(FrameContext& frame) override
	{
		TRACE_FUNCTION();

		cv::imshow("Frame", frame.mat);

		char c=(char)cv::waitKey(5);
		if(c==27)
		{
			kill_child_processes();
			exit(0);
		}
	}
};

class ObjectTracker : public FrameCvDisplayer
{
public:
	void OnFrame(FrameContext& frame) override
	{
		TRACE_FUNCTION();
		// if (not frame.cv_faces.empty())
		// {
			frame.dudesSquad.UpdateDudes(frame.mat, frame.cv_faces);
			frame.cv_faces.clear();
		// }
		// else
		// {
		// 	frame.dudesSquad.TrackingUpdate(frame.mat);
		// }
		FrameCvDisplayer::OnFrame(frame);
	}
};

class ObjectDetector : public ObjectTracker
{
public:
	ObjectDetector() try : net{"./"}, cnt{0}
	{
	}
	catch(dlib::serialization_error& e)
	{
		ERR("You need dlib's default face landmarking model file to run this example.");
		ERR("You can get it from the following URL: ");
		ERR("   http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2");
		ERR("%s", e.what());
	}

	void OnFrame(FrameContext& frame) override
	{
		TRACE_FUNCTION();
		if (cnt++ % DETECTION_PERIOD == 0)
		{
			//frame.faces = net.detector(frame.matrix);
			frame.cv_faces = net.cv_detector(frame.mat);
		}
		//INFO("ObjectDetector: faces=%zu", frame.faces.size());
		ObjectTracker::OnFrame(frame);
		//FrameCvDisplayer::OnFrame(frame);
	}

private:
	networks net;
	uint32_t cnt;
};

class FramePreprocessor : public ObjectDetector
{
public:
	void OnFrame(FrameContext& frame) override
	{
		ObjectDetector::OnFrame(frame);
	}
};

class FrameProcessingChain : public FramePreprocessor
{
public:
	void OnFrame(FrameContext& frame) override
	{
		FramePreprocessor::OnFrame(frame);
	}
};

static void on_frame_event()
{
	static auto ts = std::chrono::steady_clock::now();
	static uint32_t cnt = 0;
	constexpr uint32_t FPS_MEASUREMENT_TIME = 30;

	if (cnt++ % FPS_MEASUREMENT_TIME == 0)
	{
		auto new_ts = std::chrono::steady_clock::now();
		auto delta_ms = std::chrono::duration_cast<std::chrono::milliseconds>(new_ts - ts).count();
		uint32_t fps = (uint32_t)(FPS_MEASUREMENT_TIME / ((double)delta_ms/1000));
		INFO("FPS: %u ~%3.0fms", fps, (double)delta_ms/FPS_MEASUREMENT_TIME);
		ts = new_ts;
		if (cnt == 1)
		{
			child_plot_process = fork();
			if (child_plot_process <= 0)
			{
				int ret = system("echo \"0 0\" > plot.dat");
				if (ret)
				{
					INT("Something went wrong during system call(ret=%d)", ret);
				}
				execlp("gnuplot", "gnuplot", "liveplot.gnu", NULL);
				exit(0);
			}
		}

		std::string cmd;
		cmd = "echo \"" + std::to_string(cnt) + " " + std::to_string(fps) + "\" >> plot.dat";
		int ret = system(cmd.c_str());
		if (ret)
		{
			INT("Something went wrong during system call(ret=%d)", ret);
		}
	}
}

class CaptureFrameProvider : public IFrameProvider
{
public:
	CaptureFrameProvider() = default;

	CaptureFrameProvider(int cam_idx)
	{
		Assign(cam_idx);
	}

	CaptureFrameProvider(std::string url)
	{
		Assign(url);
	}

	template<class T>
	void Assign(T src)
	{
		if constexpr (std::is_same<std::string, T>::value)
		{
			setenv("OPENCV_FFMPEG_CAPTURE_OPTIONS", "rtsp_transport;udp", 1);
		}

		m_capture.open(src);

		if (!m_capture.isOpened())
		{
			INT("Unable to connect camera");
			return;
		}

		// uint32_t w = m_capture.get(cv::CAP_PROP_FRAME_WIDTH);
		// uint32_t h = m_capture.get(cv::CAP_PROP_FRAME_HEIGHT);
		// uint32_t fps = m_capture.get(cv::CAP_PROP_FPS);
		// INFO("w=%u, h=%u, fps=%u", w,h,fps);
		m_active = true;
	}

	void RegisterHandle(IFrameHandler* handler)   { m_handlers.insert(handler); }
	void DeregisterHandle(IFrameHandler* handler)  { m_handlers.erase(handler); }

	bool                                   IsActive() const override { return m_active;}

	bool Update()
	{
		if (m_capture.read(m_frame_ctx.mat))
		{
			on_frame_event();

			// if (m_mat.cols > 800)
			// {
			//     cv::Mat resized;
			//     cv::resize(m_mat, resized, cv::Size(m_mat.cols/2, m_mat.rows/2));
			//     m_mat = resized;
			// }

			cv::Mat resized;
			cv::Size new_size = cv::Size(m_frame_ctx.mat.size().width/2, m_frame_ctx.mat.size().height/2);
			cv::resize(m_frame_ctx.mat, resized, new_size);
			m_frame_ctx.mat = resized;

			// auto cv_image = dlib::cv_image<dlib::bgr_pixel>(m_frame_ctx.mat);
			// assign_image(m_frame_ctx.matrix, cv_image);

			for(auto& handler : m_handlers)
			{
				handler->OnFrame(m_frame_ctx);
			}
			return true;
		}

		return false;
	}

private:
	cv::VideoCapture                m_capture;
	FrameContext                    m_frame_ctx;
	bool                            m_active{false};
	std::set<IFrameHandler*>        m_handlers;
};