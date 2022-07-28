#pragma once
#include <string>
#include <vector>
#include "geo.h"

namespace transport_catalogue {
	struct Stop {
		std::string name;
		geo::Coordinates coords;

		bool operator==(const Stop& other) {
			return name == other.name && coords == other.coords;
		}
	};

	inline bool operator==(const Stop& lhs, const Stop& rhs) {
		return lhs.name == rhs.name && lhs.coords == rhs.coords;
	}

	struct Bus {
		std::string name;
		std::vector<const Stop*> stops;
		bool is_roundtrip;
	};

	struct BusData {
		std::string name;
		size_t stops_count;
		size_t unique_stops_count;
		size_t real_route_length;
		double geo_route_length;
		double curvature;
	};

	struct RoutingSettings {
		int bus_wait_time;
		int bus_velocity;
	};
}