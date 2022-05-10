#pragma once

#include "transport_catalogue.h"

#include <string>
#include <vector>
#include <deque>
#include <sstream>

namespace transport_catalogue {
	namespace input_handler {

		namespace parsed {
			struct Route {
				std::vector<std::string> stop_names;
				bool circled;
			};

			struct Bus {
				std::string name;
				Route route;
			};
		}

		class Reader {
		public:
			Reader(TransportCatalogue& cat);
			void Read(std::istream& input);
			Stop ParseStop(std::string& raw_stop_string);
			parsed::Bus ParseBus(std::string& raw_bus_string);
			parsed::Route ParseRoute(std::string& raw_route_string);
			std::unordered_map<std::string, int> ParseDistances(std::string& raw_stop_string) const;
		private:
			TransportCatalogue& catalogue_;
		};
	}
}