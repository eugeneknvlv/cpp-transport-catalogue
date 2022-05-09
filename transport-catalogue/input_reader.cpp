#include "input_reader.h"
#include "transport_catalogue.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace transport_catalogue {
	namespace input_handler {

		Reader::Reader(TransportCatalogue& cat)
			: cat_(cat)
		{}

		void Reader::Read(istream& input) {
			string line, current_stop_name;
			stringstream buses, stops;
			unordered_map<string, int> distances;
			getline(input, line); // skip line with queries_count
			size_t queries_count = stoi(line);
			for (size_t i = 0; i < queries_count; i++) {
				getline(input, line);
				if (line.substr(0, 3) == "Bus"s) {
					buses << line << endl;
					continue;
				}
				cat_.AddStop(ParseStop(line));
				stops << line << endl;
			}

			while (getline(stops, line)) {
				current_stop_name = line.substr(5, line.find(':') - 5);
				for (const auto& [stop_name, distance] : ParseDistances(line)) {
					cat_.AddDistance(current_stop_name, stop_name, distance);
				}
			}

			while (getline(buses, line)) {
				auto parsed_bus = ParseBus(line);
				cat_.AddBus(parsed_bus.name, parsed_bus.route.stop_names, parsed_bus.route.circled);
			}

		}

		Stop Reader::ParseStop(string& raw_stop) {
			size_t colon_pos = raw_stop.find(':');
			size_t comma_pos = raw_stop.find(',');
			string name = raw_stop.substr(5, colon_pos - 5);
			double lat = stod(raw_stop.substr(colon_pos + 2, comma_pos - (colon_pos + 2)));
			size_t second_comma_pos = raw_stop.find(',', comma_pos + 1);
			double lng = stod(raw_stop.substr(comma_pos + 2, second_comma_pos - (comma_pos + 2)));
			return { name, {lat, lng} };
		}

		parsed::Bus Reader::ParseBus(string& raw_bus_string) {
			size_t colon_pos = raw_bus_string.find(':');
			string name = raw_bus_string.substr(4, colon_pos - 4);
			string raw_route_string = raw_bus_string.substr(colon_pos + 2);
			parsed::Route route = ParseRoute(raw_route_string);
			return { name, route };
		}

		parsed::Route Reader::ParseRoute(string& raw_route_string) {
			bool circled = false;
			vector<string> stop_names;
			if (raw_route_string.find('-') != raw_route_string.npos) {
				circled = true;
			}
			raw_route_string += " >"s;
			while (raw_route_string.find_first_of("->") != raw_route_string.npos) {
				stop_names.push_back(raw_route_string.substr(0, raw_route_string.find_first_of("->") - 1));
				raw_route_string.erase(0, raw_route_string.find_first_of("->") + 2);
			}
			return { stop_names, circled };
		}

		unordered_map<string, int> Reader::ParseDistances(string& raw_stop_string) const {
			size_t coords_end = raw_stop_string.find(',', raw_stop_string.find(',') + 1);
			if (coords_end == raw_stop_string.npos) {
				return {};
			}

			size_t dist_begin = coords_end + 2;
			unordered_map<string, int> result;
			string raw_distances = raw_stop_string.substr(dist_begin);
			raw_distances += ',';
			while (raw_distances.find(',') != raw_distances.npos) {
				size_t m_pos = raw_distances.find('m');
				int distance = stoi(raw_distances.substr(0, m_pos));
				raw_distances.erase(0, m_pos + 5); // skip " to "

				size_t comma_pos = raw_distances.find(',');
				string destination = raw_distances.substr(0, comma_pos);
				raw_distances.erase(0, comma_pos + 2);

				result.insert({ destination, distance });
			}
			return result;
		}
	}
}
