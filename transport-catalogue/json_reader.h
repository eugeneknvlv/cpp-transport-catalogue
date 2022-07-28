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
			using Router = graph::Router<double>;

			JsonReader(TransportCatalogue& catalogue)
				: catalogue_(catalogue)
			{}

			void LoadJSON(std::istream& input);
			void ProcessBaseRequests();
			void ProcessStatRequests(std::ostream& output) const;
			map_renderer::detail::RenderSettings GetRenderSettings() const;
			RoutingSettings GetRoutingSettings() const;
			std::string GetSerializationFilename() const;
			TransportCatalogue& GetCatalogue() {return catalogue_;}

		private:
			TransportCatalogue& catalogue_;
			json::Document json_document_;

			//---------------- Base requests processing ----------------//
			void ParseStopWithoutDistances(const json::Node& stop_node);
			void ParseDistance(const json::Node& stop_node);
			void ParseBus(const json::Node& bus_node);

			//---------------- Stat requests processing ----------------//
			json::Dict ProcessStopStatRequest(const json::Node& stop_node) const;
			json::Dict ProcessBusStatRequest(const json::Node& bus_node) const;
			json::Dict ProcessMapStatRequest(const json::Node& map_node) const;
			json::Dict ProcessRouteStatRequest(const json::Node& route_node, const Router& router) const;
			std::pair<int, double> ComputeRouteLength(const Bus& bus) const;
		};

	} // namespace json_handler
} // namespace transport_catalogue