#include "transport_catalogue.h"
#include <iostream>
#include <unordered_set>
using namespace std;

namespace transport_catalogue {

	void TransportCatalogue::AddStop(string& name, geo::Coordinates coords) {
		Stop& stop_in_deque = *(stops_.insert(stops_.end(), { name, coords }));
		stopname_to_stop_.insert({ stop_in_deque.name, &stop_in_deque });
	}

	void TransportCatalogue::AddBus(string& name, vector<string>& stops, bool is_circled) {
		vector<const Stop*> stop_ptrs;
		for (const auto& stop_name : stops) {
			stop_ptrs.push_back(stopname_to_stop_.at(stop_name));
		}
		Bus& bus_in_deque = *(buses_.insert(buses_.end(), { name, stop_ptrs, is_circled }));
		busname_to_bus_.insert({ bus_in_deque.name, &bus_in_deque });

		for (const auto& stop : bus_in_deque.stops) {
			(stopname_to_busnames_[stop->name]).insert(bus_in_deque.name);
		}
	}

	void TransportCatalogue::SetDistance(string from, string to, int distance) {
		pair<const Stop*, const Stop*> stop_pair = { stopname_to_stop_.at(from), stopname_to_stop_.at(to) };
		distances_.insert({ stop_pair, distance });
	}

	optional<Stop> TransportCatalogue::FindStop(string_view stop_name) const {
		if (stopname_to_stop_.count(stop_name)) {
			return *(stopname_to_stop_.at(stop_name));
		}
		return nullopt;
	}

	optional<Bus> TransportCatalogue::FindBus(string_view bus_name) const {
		if (busname_to_bus_.count(bus_name)) {
			return *(busname_to_bus_.at(bus_name));
		}
		return nullopt;
	}

	const std::map<std::string_view, const Bus*>& TransportCatalogue::GetBusnameToBusMap() const {
		return busname_to_bus_;
	}

	const std::map<std::string_view, const Stop*>& TransportCatalogue::GetStopnameToStopMap() const {
		return stopname_to_stop_;
	}

	set<string_view> TransportCatalogue::GetBusesByStop(string_view stop_name) const {
		if (!stopname_to_busnames_.count(stop_name)) {
			return {};
		}
		return stopname_to_busnames_.at(stop_name);
	}

	double TransportCatalogue::GetDistance(string_view from, string_view to) const {
		const Stop* from_stop_ptr = stopname_to_stop_.at(from);
		const Stop* to_stop_ptr = stopname_to_stop_.at(to);
		if (distances_.count({ from_stop_ptr, to_stop_ptr })) {
			return distances_.at({ from_stop_ptr, to_stop_ptr });
		}
		return distances_.at({ to_stop_ptr, from_stop_ptr });
	}

	optional<BusData> TransportCatalogue::GetBusData(string_view bus_name) const {
		if (!(FindBus(bus_name).has_value())) {
			return nullopt;
		}

		BusData result;
		Bus bus = *(FindBus(bus_name));
		size_t stops_count = 0, unique_stops_count = 0;
		double geo_route_length;
		size_t real_route_length;

		if (bus.is_roundtrip) {
			stops_count = bus.stops.size() * 2 - 1;
		}
		else {
			stops_count = bus.stops.size();
		}
		unique_stops_count = CountUniqueStops(bus);
		geo_route_length = ComputeGeoRouteLength(bus);
		real_route_length = ComputeRealRouteLength(bus);

		result = { string(bus_name), stops_count, unique_stops_count, real_route_length, geo_route_length, real_route_length / geo_route_length };
		return result;
	}

	std::vector<geo::Coordinates> TransportCatalogue::GetEveryBusPointCoordinates() const {
		std::vector<geo::Coordinates> result;
		for (const auto& [bus_name, bus_ptr] : busname_to_bus_) {
			for (const Stop* stop : bus_ptr->stops) {
				result.push_back(stop->coords);
			}
		}
		return result;
	}

	size_t TransportCatalogue::ComputeRealRouteLength(const Bus& bus) const {
		size_t result = 0;
		for (size_t i = 0; i < bus.stops.size() - 1; i++) {
			result += GetDistance(bus.stops[i]->name, bus.stops[i + 1]->name);
		}
		if (bus.is_roundtrip) {
			for (size_t i = 0; i < bus.stops.size() - 1; i++) {
				result += GetDistance(bus.stops[i + 1]->name, bus.stops[i]->name);
			}
		}
		return result;
	}

	size_t TransportCatalogue::CountUniqueStops(const Bus& bus) const {
		const vector<const Stop*> stops = bus.stops;
		size_t unique_stops_count = 0;
		unordered_set<string> unique_stops;
		for (const Stop* stop : stops) {
			if (unique_stops.insert(stop->name).second) {
				unique_stops_count++;
			}
		}
		return unique_stops_count;
	}

	double TransportCatalogue::ComputeGeoRouteLength(const Bus& bus) const {
		double result = 0;
		for (size_t i = 0; i < bus.stops.size() - 1; i++) {
			result += geo::ComputeDistance(bus.stops[i]->coords, bus.stops[i + 1]->coords);
		}

		if (bus.is_roundtrip) {
			result *= 2;
		}

		return result;
	}

}