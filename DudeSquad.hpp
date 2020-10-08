#pragma once

#include <boost/intrusive/set.hpp>
#include "global.hpp"
#include "Dude.hpp"

class DudesSquad
{
public:
	//using Dudes_t = std::map<std::string, Dude>;
	using Dudes_set_hook_t = boost::intrusive::member_hook<Dude, Dude::set_hook_t, &Dude::set_hook>;
	using Dudes_t = boost::intrusive::set<Dude, Dudes_set_hook_t, boost::intrusive::constant_time_size<false>>;

	DudesSquad() = default;

	// Dude const* GetDude(std::string& name) const
	// {
	//     auto it = m_dudes.find(name);
	//     if (it != m_dudes.end())
	//     {
	//         return &it->second;
	//     }
	//     else
	//     {
	//         return nullptr;
	//     }
	// }

	void TrackingUpdate(cv::Mat const& frame)
	{
		TRACE_FUNCTION();
		for (auto dude_it = GetDudes().begin(); dude_it != GetDudes().end();)
		{
			bool success = dude_it->TrackingUpdate(frame);
			if (!success)
			{
				Dude* dude = &*dude_it;
				dude_it++;
				Dude::Deallocate(dude);
			}
			else
			{
				dude_it++;
			}
		}
	}

	void UpdateDudes(cv::Mat& frame, std::vector<cv::Rect>& detected_faces)
	{
		TRACE_FUNCTION();

		for(auto dude_it = GetDudes().begin(); dude_it != GetDudes().end();)
		{
			if (dude_it->UpdateFace(frame, detected_faces))
			{
				DEBUG("Dude-%s is confirmed (%zu)", dude_it->GetName().c_str(), GetDudes().size());
				dude_it->reset_counter = 0;
				dude_it++;
			}
			else
			{
				if (++dude_it->reset_counter > (MAX_RESET_COUNTER*DETECTION_PERIOD))
				{
					WARN("Dude-%s is NOT found (%zu)", dude_it->GetName().c_str(), GetDudes().size());
					dude_it = GetDudes().erase_and_dispose(dude_it, [](Dude* dude){ Dude::Deallocate(dude); });
				}
				else
				{
					//WARN("Dude-%s reset counter - %u/%u", dude_it->GetName().c_str(), dude_it->reset_counter, MAX_RESET_COUNTER);
					dude_it++;
				}
			}
		}

		for (auto& face : detected_faces)
		{
			if (not face.empty())
			{
				AllocateDude(frame, face);
			}
		}
	}

	Dudes_t& GetDudes() { return m_dudes; }

private:
	Dude* AllocateDude(cv::Mat& frame, cv::Rect& detected_face)
	{
		TRACE_FUNCTION();

		Dude* dude = Dude::Allocate(frame, detected_face);
		m_dudes.insert(*dude);

		return dude;
	}

	Dudes_t         m_dudes;
};