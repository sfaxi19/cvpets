#include "models.hpp"

namespace dlib
{
template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = add_prev1<block<N,BN,1,tag1<SUBNET>>>;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down = add_prev2<avg_pool<2,2,2,2,skip1<tag2<block<N,BN,2,tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET>
using block  = BN<con<N,3,3,1,1,relu<BN<con<N,3,3,stride,stride,SUBNET>>>>>;

template <int N, typename SUBNET> using ares      = relu<residual<block,N,affine,SUBNET>>;
template <int N, typename SUBNET> using ares_down = relu<residual_down<block,N,affine,SUBNET>>;

template <typename SUBNET> using alevel0 = ares_down<256,SUBNET>;
template <typename SUBNET> using alevel1 = ares<256,ares<256,ares_down<256,SUBNET>>>;
template <typename SUBNET> using alevel2 = ares<128,ares<128,ares_down<128,SUBNET>>>;
template <typename SUBNET> using alevel3 = ares<64,ares<64,ares<64,ares_down<64,SUBNET>>>>;
template <typename SUBNET> using alevel4 = ares<32,ares<32,ares<32,SUBNET>>>;

// Face recognitino DNN
using recognition_dnn_type = loss_metric<fc_no_bias<128,avg_pool_everything<
							alevel0<
							alevel1<
							alevel2<
							alevel3<
							alevel4<
							max_pool<3,3,2,2,relu<affine<con<32,7,7,2,2,
							input_rgb_image_sized<150>
							>>>>>>>>>>>>;
// Face detection DNN:
template <long num_filters, typename SUBNET> using con5d = con<num_filters,5,5,2,2,SUBNET>;
template <long num_filters, typename SUBNET> using con5  = con<num_filters,5,5,1,1,SUBNET>;

template <typename SUBNET> using downsampler  = relu<affine<con5d<32, relu<affine<con5d<32, relu<affine<con5d<16,SUBNET>>>>>>>>>;
template <typename SUBNET> using rcon5  = relu<affine<con5<45,SUBNET>>>;

using detection_dnn_type = loss_mmod<con<1,9,9,1,1,rcon5<rcon5<rcon5<downsampler<input_rgb_image_pyramid<pyramid_down<6>>>>>>>>;
}


dlib::shape_predictor       net_shape_pred;
dlib::recognition_dnn_type  net_recognition;
dlib::frontal_face_detector net_detector;
dlib::detection_dnn_type    net_detection;
cv::dnn::Net                net_cv_detection;

descriptors_t networks::recognise(const faces_t& faces)
{
	return net_recognition(faces);
}

dlib::full_object_detection networks::shape_pred(const image_t& img, const dlib::rectangle& face)
{
	return net_shape_pred(img, face);
}

std::vector<dlib::rectangle> networks::detector(const image_t& img)
{
	return net_detector(img);
}

#define CAFFE

std::vector<cv::Rect> networks::cv_detector(cv::Mat const& inFrame)
{
	std::vector<cv::Rect> rects;

	double inScaleFactor = 1.0;
	double confidenceThreshold = 0.5;
	cv::Size new_size = cv::Size(inFrame.size().width, inFrame.size().height);

	#ifdef CAFFE
		cv::Mat inputBlob = cv::dnn::blobFromImage(inFrame, inScaleFactor, new_size);
	#else
		cv::Mat inputBlob = cv::dnn::blobFromImage(inFrame, inScaleFactor, cv::Size(inWidth, inHeight), meanVal, true, false);
	#endif
	net_cv_detection.setInput(inputBlob, "data");

	cv::Mat detection = net_cv_detection.forward("detection_out");
	cv::Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());
	for(int i = 0; i < detectionMat.rows; i++)
	{
		float confidence = detectionMat.at<float>(i, 2);
		if(confidence > confidenceThreshold)
		{
			//printf("4, confidence=%f\n", confidence);
			int x1 = static_cast<int>(detectionMat.at<float>(i, 3) * inFrame.size().width);
			int y1 = static_cast<int>(detectionMat.at<float>(i, 4) * inFrame.size().height);
			int x2 = static_cast<int>(detectionMat.at<float>(i, 5) * inFrame.size().width);
			int y2 = static_cast<int>(detectionMat.at<float>(i, 6) * inFrame.size().height);
			cv::Rect& rect = rects.emplace_back(cv::Point(x1, y1), cv::Point(x2, y2));
			cv::rectangle(inFrame, rect, cv::Scalar(0, 255, 0), 2, 4);
		}
	}
	return rects;
}

networks::networks(std::string path)
{
	dlib::deserialize(path + "/shape_predictor_68_face_landmarks.dat") >> net_shape_pred;
	dlib::deserialize(path + "/dlib_face_recognition_resnet_model_v1.dat") >> net_recognition;
//  dlib::deserialize(path + "/mmod_human_face_detector.dat") >> net_detection;

	net_detector = dlib::get_frontal_face_detector();

	const std::string caffeConfigFile = path + "/deploy.prototxt";
	const std::string caffeWeightFile = path + "/res10_300x300_ssd_iter_140000_fp16.caffemodel";

	const std::string tensorflowConfigFile = path + "/opencv_face_detector.pbtxt";
	const std::string tensorflowWeightFile = path + "/opencv_face_detector_uint8.pb";
	#ifdef CAFFE
	  net_cv_detection = cv::dnn::readNetFromCaffe(caffeConfigFile, caffeWeightFile);
	#else
	  net_cv_detection = cv::dnn::readNetFromTensorflow(tensorflowWeightFile, tensorflowConfigFile);
	#endif
}