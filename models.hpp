#pragma once

#include <dlib/dnn.h>
#include <dlib/opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <dlib/gui_widgets.h>
#include <dlib/clustering.h>
#include <dlib/string.h>
#include <dlib/image_io.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/core/types.hpp>

using image_t =        dlib::matrix<dlib::rgb_pixel>;
using descriptors_t =  std::vector<dlib::matrix<float,0,1>>;
using faces_t =        std::vector<dlib::matrix<dlib::rgb_pixel>>;

class networks
{
public:
	networks(std::string path);
	~networks(){}

	descriptors_t                 recognise(const faces_t& faces);
	dlib::full_object_detection   shape_pred(const image_t& img, const dlib::rectangle& dets);
	std::vector<dlib::rectangle>  detector(const image_t& img);
	std::vector<cv::Rect>         cv_detector(cv::Mat const& inFrame);

};


static cv::Rect dlibRectangleToOpenCV(dlib::rectangle r)
{
	return cv::Rect(cv::Point2i(r.left(), r.top()), cv::Point2i(r.right() + 1, r.bottom() + 1));
}

static dlib::rectangle openCVRectToDlib(cv::Rect const& r)
{
	return dlib::rectangle((long)r.tl().x, (long)r.tl().y, (long)r.br().x - 1, (long)r.br().y - 1);
}