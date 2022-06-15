#pragma once
#include "json.h"
#include "json_builder.h"
#include "transport_catalogue.h"
#include "domain.h"
#include "map_renderer.h"
#include "router.h"

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
				, router_(catalogue_.GetGraphConstRef())
				, input_(input)
				, output_(output)
			{}

			void LoadJSON();
			void ProcessBaseRequests();
			void ProcessStatRequests() const;
			map_renderer::detail::RenderSettings GetRenderSettings() const;
			RoutingSettings GetRoutingSettings() const;

		private:
			TransportCatalogue& catalogue_;
			graph::Router<double> router_;
			json::Document json_document_;
			std::istream& input_;
			std::ostream& output_;

			//---------------- Base requests processing ----------------//
			void ParseStopWithoutDistances(const json::Node& stop_node);
			void ParseDistance(const json::Node& stop_node);
			void ParseBus(const json::Node& bus_node);

			//---------------- Stat requests processing ----------------//
			json::Dict ProcessStopStatRequest(const json::Node& stop_node) const;
			json::Dict ProcessBusStatRequest(const json::Node& bus_node) const;
			json::Dict ProcessMapStatRequest(const json::Node& map_node) const;
			json::Dict ProcessRouteStatRequest(const json::Node& route_node) const;
			std::pair<int, double> ComputeRouteLength(const Bus& bus) const;
		};

	} // namespace json_handler
} // namespace transport_catalogue