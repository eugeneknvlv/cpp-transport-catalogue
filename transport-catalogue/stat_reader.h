#pragma once

#include "transport_catalogue.h"

#include <optional>

namespace transport_catalogue {
	namespace statistics {

		class StatReader {
		public:
			StatReader(TransportCatalogue& cat, std::istream& input, std::ostream& out);
			void ProcessQueries() const;
			void OutputBusInfo(const Bus& bus) const;
			void OutputStopInfo(Stop stop) const;
			std::pair<int, double> ComputeRouteLength(const Bus& bus) const;

		private:
			TransportCatalogue& cat_;
			std::istream& input_;
			std::ostream& output_;
		};

	}
}