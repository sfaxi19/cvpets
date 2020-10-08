#pragma once

#include <dlib/image_processing.h>

#include "global.hpp"
#include "models.hpp"

inline dlib::rectangle resize(dlib::rectangle& rect)
{
	long new_height = rect.height() * SCALE_FACTOR;
	long new_width =  rect.width() * SCALE_FACTOR;
	long new_left = rect.left() + (rect.width()-new_width)/2;
	long new_top = rect.top() + (rect.height()-new_height)/2;

	return dlib::rectangle(new_left, new_top, new_left + new_width, new_top + new_height);
}

inline cv::Rect resize(cv::Rect& rect)
{
	long new_height = rect.height * SCALE_FACTOR;
	long new_width =  rect.width * SCALE_FACTOR;
	long new_x = rect.x + (rect.width-new_width)/2;
	long new_y = rect.y + (rect.height-new_height)/2;

	return cv::Rect(new_x, new_y, new_width, new_height);
}

inline std::string getName(std::string filename)
{
	const size_t last_slash_idx = filename.find_last_of("\\/");
	if (std::string::npos != last_slash_idx)
	{
		filename.erase(0, last_slash_idx + 1);
	}

	// Remove extension if present.
	const size_t period_idx = filename.rfind('.');
	if (std::string::npos != period_idx)
	{
		filename.erase(period_idx);
	}
	return filename;
}

inline std::vector<dlib::matrix<float,0,1>> get_face_descriptors(networks& net, dlib::matrix<dlib::rgb_pixel> cimg)
{
	std::vector<dlib::matrix<dlib::rgb_pixel>>  faces;
	std::vector<dlib::full_object_detection>    shapes;

	auto detected_faces = net.detector(cimg);

	for (auto face : detected_faces)
	{
		auto shape = net.shape_pred(cimg, face);

		dlib::matrix<dlib::rgb_pixel> face_chip;
		extract_image_chip(cimg, get_face_chip_details(shape, 150, 0.25), face_chip);
		faces.push_back(std::move(face_chip));

		shapes.push_back(shape);
	}

	if (faces.size() == 0)
	{
		std::cout << "No faces found in image!" << std::endl;
		exit(1);
	}

	std::vector<dlib::matrix<float,0,1>> face_descriptors = net.recognise(faces);
	// for (int i = 0; i < face_descriptors.size(); i++)
	// {
	//     cout << "face descriptor[" << std::to_string(i) << "]: " << dlib::trans(face_descriptors[i]) << endl;
	// }

	return face_descriptors;
}