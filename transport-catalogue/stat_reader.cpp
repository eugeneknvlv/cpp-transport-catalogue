#include "stat_reader.h"

#include <iostream>
#include <string>
#include <unordered_set>

using namespace std;

namespace transport_catalogue {
	namespace statistics {

		StatReader::StatReader(TransportCatalogue& catalogue, istream& input, ostream& out)
			: catalogue_(catalogue)
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
					OutputBusInfo(bus_name);
				}
				else if (key_word == "Stop"s) {
					stop_name = line.substr(5);
					OutputStopInfo(stop_name);
				}
			}
		}

		void StatReader::OutputBusInfo(const string& bus_name) const {
			if (!(catalogue_.GetBusData(bus_name).has_value())) {
				cout << "Bus " << bus_name << ": not found" << endl;
				return;
			}

			BusData bus_data = *(catalogue_.GetBusData(bus_name));
			output_ << "Bus " << bus_data.name << ": " << bus_data.stops_count << " stops on route, "
				<< bus_data.unique_stops_count << " unique stops, " << static_cast<double>(bus_data.real_route_length)
				<< " route length, " << bus_data.curvature << " curvature" << endl;
		} 

		void StatReader::OutputStopInfo(const string& stop_name) const {
			if (!(catalogue_.FindStop(stop_name).has_value())) {
				cout << "Stop " << stop_name << ": not found" << endl;
				return;
			}

			output_ << "Stop " << stop_name << ": ";
			set<string_view> buses = catalogue_.GetBusesByStop(stop_name);
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