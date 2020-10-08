#pragma once

#include <boost/intrusive/set.hpp>

#include "global.hpp"
#include "models.hpp"
#include "FrameProviders.hpp"
#include "utils.hpp"

class Dude
{
public:
	static Dude* Allocate(cv::Mat& frame, cv::Rect& detected_face)
	{
		return new Dude(frame, detected_face);
	}

	static void Deallocate(Dude* dude)
	{
		delete dude;
	}

	Dude(Dude const&) = delete;
	Dude& operator=(Dude const&) = delete;

	Dude(Dude &&) = default;
	Dude& operator=(Dude &&) = default;

	//dlib::drectangle GetFacePosition() { return m_tracker.get_position(); }
	cv::Rect GetFacePosition() { return m_tracker_predict; }

	bool TrackingUpdate(cv::Mat const& frame)
	{
		TRACE_FUNCTION();

		cv::Rect2d tmp;
		bool res = m_tracker2->update(frame, tmp);
		//DEBUG("update=%d", res);
		m_tracker_predict = tmp;

		cv::rectangle(frame, m_tracker_predict, cv::Scalar(0, 0, 255), 2, 4);
		int font = cv::FONT_HERSHEY_SCRIPT_SIMPLEX;
		double fontScale = 1;
		int thickness = 1;

		cv::putText(frame, GetName().c_str(), m_tracker_predict.tl(), font, fontScale, cv::Scalar(0, 0, 255), thickness);

		return res;
	}

	// face detector
	// face tracker
	bool UpdateFace(cv::Mat& frame, std::vector<cv::Rect>& detected_faces)
	{
		TRACE_FUNCTION();

		for (auto& face : detected_faces)
		{
			auto inter_rect = face & m_tracker_predict;
			double area = inter_rect.area();
			double area1 = face.area();
			double area2 = m_tracker_predict.area();
			bool found = area > std::min(area1, area2)/2;

			if (found)
			{
				m_tracker2.reset();
				m_tracker2 = cv::TrackerCSRT::create();
				m_tracker_predict = face; //resize(face);
				m_tracker2->init(frame, m_tracker_predict);
				//m_tracker.start_track(face, tracking_face);
				face = cv::Rect();
				return true;
			}
		}

		//m_tracker.update(GetProvider().GetArrayImage());

		TrackingUpdate(frame);
		return false;
	}

	std::string const& GetName() const { return m_name; }

	using set_hook_t = boost::intrusive::set_member_hook<boost::intrusive::link_mode<boost::intrusive::auto_unlink>>;
	set_hook_t set_hook;
	friend bool operator< (const Dude &a, const Dude &b)  {  return a.m_name < b.m_name;  }
	friend bool operator> (const Dude &a, const Dude &b)  {  return a.m_name > b.m_name;  }
	friend bool operator== (const Dude &a, const Dude &b) {  return a.m_name == b.m_name; }


	uint32_t reset_counter{0};
private:
	Dude(cv::Mat& frame, cv::Rect& detected_face)
		: m_tracker2{cv::TrackerCSRT::create()}
	{
		m_name = std::to_string(++dude_cnt);

		INFO("Dude-%s is detected!", m_name.c_str());

		m_tracker_predict = detected_face;//resize(detected_face);
		m_tracker2->init(frame, m_tracker_predict);

		//m_tracker.start_track(GetProvider().GetArrayImage(), resize(detected_face));
	}

	~Dude()
	{
		ERR("Dude-%s is lost!", m_name.c_str());
	}

	std::string               m_name;
	dlib::correlation_tracker m_tracker;
	cv::Ptr<cv::Tracker>      m_tracker2;
	cv::Rect                  m_tracker_predict;
	bool                      m_started{false};

	static size_t             dude_cnt;
};

size_t Dude::dude_cnt = 0;