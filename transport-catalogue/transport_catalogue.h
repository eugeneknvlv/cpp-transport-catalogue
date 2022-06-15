#pragma once

#include "geo.h"
#include "domain.h"
#include "graph.h"
#include "router.h"

#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <unordered_map>
#include <map>
#include <optional>
#include <set>

namespace transport_catalogue {

	namespace detail {
		struct StopPtrPairHahser {
			size_t operator() (const std::pair<const Stop*, const Stop*>& stop_ptr_pair) const {
				return h_ptr(stop_ptr_pair.first) * 3 + h_ptr(stop_ptr_pair.second);
			}

		private:
			std::hash<const void*> h_ptr;
		};
	}

	class TransportCatalogue {
	public:
		using Route = graph::Router<double>::RouteInfo;
		using Graph = graph::DirectedWeightedGraph<double>;

		void AddStop(const std::string& name, geo::Coordinates coords);
		void AddBus(const std::string& name, const std::vector<std::string>& stops, bool circled);
		void SetDistance(const std::string& from, const std::string& to, int distance);
		void SetRoutingSettings(RoutingSettings rt);
		void BuildGraph(); 
		std::optional<Stop> FindStop(std::string_view name) const;
		std::optional<Bus> FindBus(std::string_view name) const;
		const std::map<std::string_view, const Bus*>& GetBusnameToBusMap() const;
		std::set<std::string_view> GetBusesByStop(std::string_view stop_name) const;
		double GetDistance(std::string_view from, std::string_view to) const;
		std::optional<BusData> GetBusData(std::string_view bus_name) const;
		std::vector<geo::Coordinates> GetEveryBusPointCoordinates() const;
		int GetEdgeSpanCount(graph::EdgeId id) const;
		std::string_view GetStopNameByVertexId(graph::VertexId id) const;
		graph::VertexId GetVertexIdByStopName(std::string_view stop_name) const;
		std::pair<std::string_view, std::string_view> GetEdgeStops(graph::EdgeId id) const;
		std::string_view GetEdgeBusName(graph::EdgeId id) const;
		double GetEdgeWeight(graph::EdgeId id) const;
		int GetBusWaitingTime() const;
		const Graph& GetGraphConstRef() const;

	private:
		std::deque<Stop> stops_;
		std::map<std::string_view, const Stop*> stopname_to_stop_;
		std::deque<Bus> buses_;
		std::map<std::string_view, const Bus*> busname_to_bus_;
		std::unordered_map<std::string_view, std::set<std::string_view>> stopname_to_busnames_;
		std::unordered_map<std::pair<const Stop*, const Stop*>, double, detail::StopPtrPairHahser> distances_;
		RoutingSettings routing_settings_;
		Graph graph_;

		size_t ComputeRealRouteLength(const Bus& bus) const;
		size_t CountUniqueStops(const Bus& bus) const;
		double ComputeGeoRouteLength(const Bus& bus) const;
	};

}