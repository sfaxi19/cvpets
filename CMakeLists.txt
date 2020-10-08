cmake_minimum_required(VERSION 3.0)

project(cvpets VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-O3")

add_executable(cvpets main.cpp models.cpp)
add_executable(tracking_test tracking.cpp models.cpp)
configure_file(
	models/shape_predictor_68_face_landmarks.dat
	shape_predictor_68_face_landmarks.dat
	COPYONLY)
configure_file(
	models/dlib_face_recognition_resnet_model_v1.dat
	dlib_face_recognition_resnet_model_v1.dat
	COPYONLY)

configure_file(
	models/opencv_face_detector/deploy.prototxt
	deploy.prototxt
	COPYONLY)
configure_file(
	models/opencv_face_detector/res10_300x300_ssd_iter_140000_fp16.caffemodel
	res10_300x300_ssd_iter_140000_fp16.caffemodel
	COPYONLY)

configure_file(
	models/opencv_face_detector/opencv_face_detector.pbtxt
	opencv_face_detector.pbtxt
	COPYONLY)
configure_file(
	models/opencv_face_detector/opencv_face_detector_uint8.pb
	opencv_face_detector_uint8.pb
	COPYONLY)

#find_package(DLIB REQUIRED)
#include_directories(${DLIB_INCLUDE_DIRECTORY})
find_package(Boost)

if (Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIR})
	ADD_DEFINITIONS( "-DHAS_BOOST" )
endif()
target_link_libraries(cvpets dlib pthread dl X11 opencv_videoio opencv_highgui opencv_core jpeg png opencv_imgproc opencv_tracking opencv_dnn ${Boost_LIBRARIES})
target_link_libraries(tracking_test dlib pthread dl X11 opencv_videoio opencv_highgui opencv_core jpeg png opencv_imgproc opencv_tracking opencv_dnn ${Boost_LIBRARIES})
