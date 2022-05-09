#pragma once

#include "geo.h"

#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <unordered_map>
#include <optional>
#include <set>

namespace transport_catalogue {

	struct Stop {
		std::string name;
		detail::Coordinates coords;
	};

	namespace detail {
		struct StopPtrPairHahser {
			size_t operator() (const std::pair<const Stop*, const Stop*>& stop_ptr_pair) const {
				return h_ptr(stop_ptr_pair.first) * 3 + h_ptr(stop_ptr_pair.second);
			}

		private:
			std::hash<const void*> h_ptr;
		};
	}

	struct Bus {
		std::string name;
		std::vector<const Stop*> stops;
		bool circled;
	};

	class TransportCatalogue {
	public:
		void AddStop(Stop stop);
		void AddBus(std::string name, std::vector<std::string>& stops, bool circled);
		void AddDistance(std::string stop1_name, std::string stop2_name, int distance);
		std::optional<Stop> FindStop(const std::string& name) const;
		std::optional<Bus> FindBus(const std::string& name) const;
		std::set<std::string_view> GetBusesByStop(const std::string& stop_name) const;
		double GetDistance(const std::string& stop1_name, const std::string& stop2_name) const;

	private:
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
		std::unordered_map<std::string_view, std::set<std::string_view>> stopname_to_busnames_;
		std::unordered_map<std::pair<const Stop*, const Stop*>, double, detail::StopPtrPairHahser> distances_;
	};

}