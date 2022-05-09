#include "transport_catalogue.h"
#include <iostream>
using namespace std;

namespace transport_catalogue {

	void TransportCatalogue::AddStop(Stop stop) {
		Stop& stop_in_deque = *(stops_.insert(stops_.end(), stop));
		stopname_to_stop_.insert({ stop_in_deque.name, &stop_in_deque });

	}

	void TransportCatalogue::AddBus(string name, vector<string>& stops, bool circled) {
		vector<const Stop*> stop_ptrs;
		for (const auto& stop_name : stops) {
			stop_ptrs.push_back(stopname_to_stop_.at(stop_name));
		}
		Bus& bus_in_deque = *(buses_.insert(buses_.end(), { name, stop_ptrs, circled }));
		busname_to_bus_.insert({ bus_in_deque.name, &bus_in_deque });

		for (const auto& stop : bus_in_deque.stops) {
			(stopname_to_busnames_[stop->name]).insert(bus_in_deque.name);
		}
	}

	void TransportCatalogue::AddDistance(string stop1_name, string stop2_name, int distance) {
		pair<const Stop*, const Stop*> stop_pair = { stopname_to_stop_.at(stop1_name), stopname_to_stop_.at(stop2_name) };
		distances_.insert({ stop_pair, distance });
	}

	optional<Stop> TransportCatalogue::FindStop(const string& stop_name) const {
		if (stopname_to_stop_.count(stop_name)) {
			return *(stopname_to_stop_.at(stop_name));
		}
		return nullopt;
	}

	optional<Bus> TransportCatalogue::FindBus(const string& bus_name) const {
		if (busname_to_bus_.count(bus_name)) {
			return *(busname_to_bus_.at(bus_name));
		}
		return nullopt;
	}


	set<string_view> TransportCatalogue::GetBusesByStop(const string& stop_name) const {
		if (!stopname_to_busnames_.count(stop_name)) {
			return {};
		}
		return stopname_to_busnames_.at(stop_name);
	}

	double TransportCatalogue::GetDistance(const string& stop1_name, const string& stop2_name) const {
		const Stop* stop1 = stopname_to_stop_.at(stop1_name);
		const Stop* stop2 = stopname_to_stop_.at(stop2_name);
		if (distances_.count({ stop1, stop2 })) {
			return distances_.at({ stop1, stop2 });
		}
		return distances_.at({ stop2, stop1 });
	}

}