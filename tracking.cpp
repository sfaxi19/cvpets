#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_io.h>
#include <dlib/dir_nav.h>
#include "global.hpp"
#include "models.hpp"

using namespace dlib;
using namespace std;

bool object_initialization(rectangle& tracking_obj, image_window& win)
{
	point p1,p2;

	win.get_next_double_click(p1);
	win.get_next_double_click(p2);

	tracking_obj = rectangle(p1,p2);

	win.add_overlay(dlib::image_window::overlay_rect(tracking_obj, dlib::rgb_pixel(255,0,0), "Object"));

	unsigned long key;
	bool is_printable;

	while(true)
	{
		win.get_next_keypress(key, is_printable);

		printf("%u\n", key);

		if (key == 10) { return true; }
		if (key == 32) { return false; }
	}
}

int main(int argc, char** argv) try
{
	cv::VideoCapture cap(2);

	if (!cap.isOpened())
	{
		std::cerr << "Unable to connect to camera" << std::endl;
		return 1;
	}

	image_window win;

	cv::Mat temp;

	rectangle tracking_obj;

	correlation_tracker tracker;
	cv::Ptr<cv::Tracker> tracker2 = cv::TrackerCSRT::create(); //TrackerCSRT - very nice!

	bool init = true;

	while(!win.is_closed())
	{
		if (!cap.read(temp))
		{
			break;
		}

		dlib::cv_image<dlib::bgr_pixel> image(temp);
		dlib::matrix<dlib::rgb_pixel> cimg;
		dlib::array2d<uint8_t> img;

		assign_image(cimg, image);
		assign_image(img, image);

		// Display it all on the screen
		win.clear_overlay();
		win.set_image(cimg);

		if (init)
		{
			bool res = object_initialization(tracking_obj, win);

			if (res)
			{
				printf("start_track\n");
				//tracker.start_track(img, tracking_obj);
				tracker2->init(temp, dlibRectangleToOpenCV(tracking_obj));
				init = false;
				continue;
			}
			else
			{
				continue;
			}
		}

		cv::Rect2d result;

		//tracker.update(img);

		tracker2->update(temp, result);

		win.add_overlay(openCVRectToDlib(result));
	}
}
catch (std::exception& e)
{
	cout << e.what() << endl;
}


