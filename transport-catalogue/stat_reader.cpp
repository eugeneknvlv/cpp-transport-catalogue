#include "stat_reader.h"

#include <iostream>
#include <string>
#include <unordered_set>

using namespace std;

namespace transport_catalogue {
	namespace statistics {

		StatReader::StatReader(TransportCatalogue& cat, istream& input, ostream& out)
			: cat_(cat)
			, input_(input)
			, output_(out)
		{}

		void StatReader::ProcessQueries() const {
			size_t queries_count;
			string line, key_word, bus_name, stop_name;
			input_ >> queries_count;
			getline(input_, line);
			for (size_t i = 0; i < queries_count; i++) {
				getline(input_, line);
				key_word = line.substr(0, line.find(' '));
				if (key_word == "Bus"s) {
					bus_name = line.substr(4);
					if (cat_.FindBus(bus_name).has_value()) {
						OutputBusInfo(*(cat_.FindBus(bus_name)));
					}
					else {
						output_ << "Bus " << bus_name << ": not found" << endl;
					}
				}
				else if (key_word == "Stop"s) {
					stop_name = line.substr(5);
					if (cat_.FindStop(stop_name).has_value()) {
						OutputStopInfo(*(cat_.FindStop(stop_name)));
					}
					else {
						output_ << "Stop " << stop_name << ": not found" << endl;
					}
				}
			}
		}

		size_t CountUniqueStops(const vector<const Stop*>& stops) {
			size_t unique_stops_count = 0;
			unordered_set<string> unique_stops;
			for (const Stop* stop : stops) {
				if (unique_stops.insert(stop->name).second) {
					unique_stops_count++;
				}
			}
			return unique_stops_count;
		}

		void StatReader::OutputBusInfo(const Bus& bus) const {
			size_t stops_count = 0, unique_stops_count = 0;
			double geo_route_length;
			int real_route_length;
			if (bus.circled) {
				stops_count = bus.stops.size() * 2 - 1;
			}
			else {
				stops_count = bus.stops.size();
			}
			unique_stops_count = CountUniqueStops(bus.stops);
			geo_route_length = ComputeRouteLength(bus).second;
			real_route_length = ComputeRouteLength(bus).first;
			double curvature = real_route_length / geo_route_length;
			output_ << "Bus " << bus.name << ": " << stops_count << " stops on route, "
				<< unique_stops_count << " unique stops, " << static_cast<double>(real_route_length)
				<< " route length, " << curvature << " curvature" << endl;
		}

		pair<int, double> StatReader::ComputeRouteLength(const Bus& bus) const {
			double geo_distance = 0;
			int real_distance = 0;
			for (size_t i = 0; i < bus.stops.size() - 1; i++) {
				geo_distance += detail::ComputeDistance(bus.stops[i]->coords, bus.stops[i + 1]->coords);
				real_distance += cat_.GetDistance(bus.stops[i]->name, bus.stops[i + 1]->name);
			}
			if (bus.circled) {
				for (size_t i = 0; i < bus.stops.size() - 1; i++) {
					real_distance += cat_.GetDistance(bus.stops[i + 1]->name, bus.stops[i]->name);
				}
				geo_distance *= 2;
			}

			return { real_distance, geo_distance };
		}

		void StatReader::OutputStopInfo(Stop stop) const {
			output_ << "Stop " << stop.name << ": ";
			set<string_view> buses = cat_.GetBusesByStop(stop.name);
			if (buses.empty()) {
				output_ << "no buses" << endl;
				return;
			}

			output_ << "buses";
			for (string_view bus : buses) {
				output_ << " " << bus;
			}
			output_ << endl;
		}
	}
}