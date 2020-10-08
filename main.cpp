// g++ -std=c++17 -O3 rectest.cpp -ldlib -lpthread -lX11 -lopencv_videoio -lopencv_highgui -lopencv_core
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_io.h>
#include <dlib/dir_nav.h>
#include <thread>
#include <csignal>
#include <boost/stacktrace.hpp>

#include "FrameProviders.hpp"

void signal_handler(int signal)
{
	std::cout << "\nSignal code=" << signal << std::endl;
	std::cout << boost::stacktrace::stacktrace() << std::endl;
	kill_child_processes();
	exit(1);
}


int main(int argc, char** argv)
{
	std::signal(SIGINT,  signal_handler);
	std::signal(SIGILL,  signal_handler);
	std::signal(SIGABRT, signal_handler);
	std::signal(SIGFPE,  signal_handler);
	std::signal(SIGSEGV, signal_handler);
	std::signal(SIGTERM, signal_handler);

	using namespace std::chrono_literals;

	try
	{
		CaptureFrameProvider frameProvider;
		FrameProcessingChain processingChain;

		frameProvider.RegisterHandle(&processingChain);

		if (argc > 2) INT("Bad number of input arguments");

		if (argc == 2)
		{
			// ./rectest rtsp://login:password@192.168.1.127:554/onvif1
			// ./rectest video.avi
			// ./rectest imgs/%06d.jpg
			frameProvider.Assign(argv[1]);
		}
		else if (argc == 1)
		{
			int cam_idx = 0;
			INFO("Camera list:");
			int ret = system("ls -l /dev/video* | awk -F \"/\" \'{print \" - \" $3}\'");
			if (ret)
			{
				INT("Something went wrong during system call(ret=%d)", ret);
			}
			else
			{
				INFO("Please choose the camera number: ");
				std::cin >> cam_idx;
				frameProvider.Assign(cam_idx);
			}
		}

		if (not frameProvider.IsActive()) INT("FrameProvider is not active");


		while(frameProvider.Update()) { }
	}
	catch(std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

	kill_child_processes();
}

