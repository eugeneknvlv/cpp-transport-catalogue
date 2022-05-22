#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "domain.h"
#include "map_renderer.h"

#include <vector>
#include <string>

namespace transport_catalogue {
	namespace json_handler {

		struct ParsedBus {
			std::string name;
			std::vector<std::string> stop_names;
			bool is_roundtrip;
		};

		class JsonReader {
		public:
			JsonReader(TransportCatalogue& catalogue, std::istream& input, std::ostream& output)
				: catalogue_(catalogue)
				, input_(input)
				, output_(output)
			{}

			void LoadJSON();
			void ProcessBaseRequests() const;
			void ProcessStatRequests() const;
			map_renderer::detail::RenderSettings GetRenderSettings() const;

		private:
			TransportCatalogue& catalogue_;
			json::Document json_document_;
			std::istream& input_;
			std::ostream& output_;

			//void ParseJsonDocument() const;

			//---------------- Base requests processing ----------------//
			Stop ParseStopWithoutDistances(const json::Node& stop_node) const;
			ParsedBus ParseBus(const json::Node& bus_node) const;

			//---------------- Stat requests processing ----------------//
			json::Dict ProcessStopStatRequest(const json::Node& stop_node) const;
			json::Dict ProcessBusStatRequest(const json::Node& bus_node) const;
			json::Dict ProcessMapStatRequest(const json::Node& map_node) const;
			std::pair<int, double> ComputeRouteLength(const Bus& bus) const;
		};

	} // namespace json_handler
} // namespace transport_catalogue