#pragma once

#include "transport_catalogue.h"

#include <optional>

namespace transport_catalogue {
	namespace statistics {

		class StatReader {
		public:
			StatReader(TransportCatalogue& cat, std::istream& input, std::ostream& out);
			void ProcessQueries() const;
			void OutputBusInfo(const std::string& bus_name) const;
			void OutputStopInfo(const std::string& stop_name) const;

		private:
			TransportCatalogue& catalogue_;
			std::istream& input_;
			std::ostream& output_;
		};

	}
}