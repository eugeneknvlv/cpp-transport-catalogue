#include "json_reader.h"


#include <set>
#include <string_view>
#include <optional>
#include <unordered_set>
#include <sstream>
#include <fstream>

using namespace std::literals;

namespace transport_catalogue {
	namespace json_handler {

		void JsonReader::LoadJSON(std::istream& input) {
			json_document_ = json::Load(input);
			//ParseJsonDocument();
		}

		// ------------------------ Base requests processing ------------------------ //
		void JsonReader::ProcessBaseRequests(/*const json::Node& base_root*/) {
			json::Array requests_array = json_document_.GetRoot().AsMap().at("base_requests"s).AsArray();

			// Loop 1 - process stops
			for (const json::Node& single_request : requests_array) {   // [ {...}, {...}, {...}, ... ]
				std::string request_type = ((single_request.AsMap()).at("type"s)).AsString();
				if (request_type == "Stop"s) {
					ParseStopWithoutDistances(single_request);
				}
			}

			// Loop 2 - process road_distances between stops
			for (const json::Node& single_request : requests_array) {   // [ {...}, {...}, {...}, ... ]
				std::string request_type = ((single_request.AsMap()).at("type"s)).AsString();
				if (request_type == "Stop"s) {
					ParseDistance(single_request);
				}
			}

			// Loop 3 - process buses
			for (const json::Node& single_request : requests_array) {   // [ {...}, {...}, {...}, ... ]
				std::string request_type = ((single_request.AsMap()).at("type"s)).AsString();
				if (request_type == "Bus"s) {
					ParseBus(single_request);
				}
			}

			catalogue_.SetRenderSettings(GetRenderSettings());
			catalogue_.SetRoutingSettings(GetRoutingSettings());
			catalogue_.BuildGraph();
		}

		void JsonReader::ParseStopWithoutDistances(const json::Node& stop_node) {
			json::Dict stop_info_map = stop_node.AsMap();
			std::string stop_name = stop_info_map.at("name"s).AsString();
			geo::Coordinates coordinates = { stop_info_map.at("latitude"s).AsDouble(), stop_info_map.at("longitude"s).AsDouble() };
			catalogue_.AddStop(stop_name, coordinates);
		}

		void JsonReader::ParseDistance(const json::Node& stop_node) {
			std::string current_stop_name = stop_node.AsMap().at("name"s).AsString();
			json::Dict road_distances = stop_node.AsMap().at("road_distances"s).AsMap();
			for (const auto& [stop_name, dist_node] : road_distances) {
				catalogue_.SetDistance(current_stop_name, stop_name, dist_node.AsInt());
			}
		}

		void JsonReader::ParseBus(const json::Node& bus_node) {
			json::Dict bus_node_as_map = bus_node.AsMap();
			std::string name = bus_node_as_map.at("name"s).AsString();
			bool is_roundtrip = bus_node_as_map.at("is_roundtrip"s).AsBool();
			std::vector<std::string> stop_names;
			for (const json::Node& stop_node : bus_node_as_map.at("stops"s).AsArray()) {
				stop_names.push_back(stop_node.AsString());
			}
			catalogue_.AddBus(name, stop_names, is_roundtrip);
		}
		//-------------------- Base requests processing end ---------------------//


		//-------------------- Stat requests processing ------------------------//
		void JsonReader::ProcessStatRequests(std::ostream& output) const {
			json::Array requests_array = json_document_.GetRoot().AsMap().at("stat_requests"s).AsArray();
			json::Builder builder{};
			json::ArrayContext arr_ctx = builder.StartArray();
			const TransportCatalogue::Graph& gr = catalogue_.GetGraphConstRef();
			graph::Router router(gr);

			for (const json::Node& single_request : requests_array) {
				std::string request_type = ((single_request.AsMap()).at("type"s)).AsString();
				if (request_type == "Stop"s) {
					arr_ctx.Value(ProcessStopStatRequest(single_request));
				}
				else if (request_type == "Bus"s) {
					arr_ctx.Value(ProcessBusStatRequest(single_request));
				}
				else if (request_type == "Map"s) {
					arr_ctx.Value(ProcessMapStatRequest(single_request));
				}
				else if (request_type == "Route"s) {
					arr_ctx.Value(ProcessRouteStatRequest(single_request, router));
				} 
			}
			json::Builder result = arr_ctx.EndArray();
			json::Print(json::Document(result.Build()), output);
		}

		json::Dict JsonReader::ProcessStopStatRequest(const json::Node& stop_node) const {
			int request_id = stop_node.AsMap().at("id"s).AsInt();
			std::string name = stop_node.AsMap().at("name"s).AsString();
			if (!catalogue_.FindStop(name)) {
				return { {"request_id"s, json::Node(request_id)}, {"error_message"s, json::Node("not found"s)} };
			}
			std::set<std::string_view> buses_passing_through_current_stop = catalogue_.GetBusesByStop(name);
			json::Array buses_array;
			for (std::string_view bus_sv : buses_passing_through_current_stop) {
				buses_array.push_back(json::Node(std::string(bus_sv)));
			}
			return { {"buses"s, json::Node(buses_array)}, {"request_id"s, json::Node(request_id)} };
		}

		size_t CountUniqueStops(const std::vector<const Stop*>& stops) {
			size_t unique_stops_count = 0;
			std::unordered_set<std::string> unique_stops;
			for (const Stop* stop : stops) {
				if (unique_stops.insert(stop->name).second) {
					unique_stops_count++;
				}
			}
			return unique_stops_count;
		}

		json::Dict JsonReader::ProcessBusStatRequest(const json::Node& bus_node) const {
			int request_id = bus_node.AsMap().at("id"s).AsInt();
			std::string name = bus_node.AsMap().at("name"s).AsString();
			std::optional<Bus> bus = catalogue_.FindBus(name);
			if (!bus.has_value()) {
				return { {"request_id"s, json::Node(request_id)}, {"error_message"s, json::Node("not found"s)} };
			}

			int stops_count;
			if (!(*bus).is_roundtrip) {
				stops_count = (*bus).stops.size() * 2 - 1;
			}
			else {
				stops_count = (*bus).stops.size();
			}
			int road_route_length = ComputeRouteLength(*bus).first;
			double geo_route_length = ComputeRouteLength(*bus).second;
			double curvature = road_route_length / geo_route_length;
			int unique_stops_count = CountUniqueStops((*bus).stops);
			return
			{
				{"request_id"s, json::Node(request_id)},
				{"stop_count"s, json::Node(stops_count)},
				{"unique_stop_count"s, json::Node(unique_stops_count)},
				{"route_length"s, json::Node(road_route_length)},
				{"curvature"s, json::Node(curvature)}
			};
		}

		json::Dict JsonReader::ProcessMapStatRequest(const json::Node& map_node) const {
			int request_id = map_node.AsMap().at("id"s).AsInt();
			map_renderer::MapRenderer map_renderer(catalogue_.GetRenderSettings(), catalogue_.GetBusnameToBusMap());
			map_renderer.SetScalingSettings(catalogue_.GetEveryBusPointCoordinates());

			std::ostringstream map_oss;
			(map_renderer.RenderMap()).Render(map_oss);

			return { {"request_id"s, request_id}, {"map"s, map_oss.str()} };
		}

		json::Dict JsonReader::ProcessRouteStatRequest(const json::Node& route_node, const Router& router) const {
			int request_id = route_node.AsMap().at("id"s).AsInt();
			
			std::string from = route_node.AsMap().at("from"s).AsString();
			size_t from_id = catalogue_.GetVertexIdByStopName(from);
			std::string to = route_node.AsMap().at("to"s).AsString();
			size_t to_id = catalogue_.GetVertexIdByStopName(to);
			std::optional<TransportCatalogue::Route> route = router.BuildRoute(from_id, to_id);
			if (!route.has_value()) {
				return { {"request_id"s, request_id}, {"error_message"s, "not found"s}};
			}
			else if ((*route).edges.empty()) {
				return
				{
					{"request_id"s, request_id},
					{"total_time"s, 0},
					{"items"s, json::Array(0)}
				};
			}

			json::Builder builder{};
			json::ArrayContext arr_ctx = builder.StartDict()
													.Key("request_id").Value(request_id)
													.Key("total_time"s).Value((*route).weight)
													.Key("items"s).StartArray();

			int bus_waiting_time = catalogue_.GetBusWaitingTime();
			std::string_view waiting_stop;
			std::string bus_name;
			double travel_time;
			int span_count;

			for (graph::EdgeId edge_id : (*route).edges) {
				waiting_stop = catalogue_.GetEdgeStops(edge_id).first;
				bus_name = catalogue_.GetEdgeBusName(edge_id);
				travel_time = catalogue_.GetEdgeWeight(edge_id) - bus_waiting_time;
				span_count = catalogue_.GetEdgeSpanCount(edge_id);
				arr_ctx.StartDict()
							.Key("type"s).Value("Wait"s)
							.Key("stop_name"s).Value(std::string(waiting_stop))
							.Key("time").Value(bus_waiting_time)
						.EndDict()
						.StartDict()
							.Key("type"s).Value("Bus"s)
							.Key("bus").Value(std::string(bus_name))
							.Key("span_count"s).Value(span_count)
							.Key("time").Value(travel_time)
						.EndDict();
			}
			return arr_ctx.EndArray().Build().AsMap();
		}

		std::pair<int, double> JsonReader::ComputeRouteLength(const Bus& bus) const {
			double geo_distance = 0;
			int real_distance = 0;
			for (size_t i = 0; i < bus.stops.size() - 1; i++) {
				geo_distance += geo::ComputeDistance(bus.stops[i]->coords, bus.stops[i + 1]->coords);
				real_distance += catalogue_.GetDistance(bus.stops[i]->name, bus.stops[i + 1]->name);
			}
			if (!bus.is_roundtrip) {
				for (size_t i = 0; i < bus.stops.size() - 1; i++) {
					real_distance += catalogue_.GetDistance(bus.stops[i + 1]->name, bus.stops[i]->name);
				}
				geo_distance *= 2;
			}

			return { real_distance, geo_distance };
		}

		//------------------- Render settings processing ---------------------//
		svg::Rgb MakeRgbFromJsonArray(const json::Array& rgb_array) {
			return
			{
				static_cast<uint8_t>((rgb_array[0]).AsInt()),
				static_cast<uint8_t>((rgb_array[1]).AsInt()),
				static_cast<uint8_t>((rgb_array[2]).AsInt())
			};
		}

		svg::Rgba MakeRgbaFromJsonArray(const json::Array& rgba_array) {
			return
			{
				static_cast<uint8_t>((rgba_array[0]).AsInt()),
				static_cast<uint8_t>((rgba_array[1]).AsInt()),
				static_cast<uint8_t>((rgba_array[2]).AsInt()),
				(rgba_array[3]).AsDouble()
			};
		}

		map_renderer::detail::RenderSettings JsonReader::GetRenderSettings() const {
			map_renderer::detail::RenderSettings result;
			json::Dict render_settings_json_map = json_document_.GetRoot().AsMap().at("render_settings"s).AsMap();
			result.width = render_settings_json_map.at("width"s).AsDouble();
			result.height = render_settings_json_map.at("height"s).AsDouble();
			result.padding = render_settings_json_map.at("padding"s).AsDouble();
			result.line_width = render_settings_json_map.at("line_width"s).AsDouble();
			result.stop_radius = render_settings_json_map.at("stop_radius"s).AsDouble();
			result.bus_label_font_size = render_settings_json_map.at("bus_label_font_size"s).AsInt();
			result.bus_label_offset =
			{
				(render_settings_json_map.at("bus_label_offset"s).AsArray()[0]).AsDouble(),
				(render_settings_json_map.at("bus_label_offset"s).AsArray()[1]).AsDouble()
			};

			result.stop_label_font_size = render_settings_json_map.at("stop_label_font_size").AsInt();
			result.stop_label_offset =
			{
				(render_settings_json_map.at("stop_label_offset"s).AsArray()[0]).AsDouble(),
				(render_settings_json_map.at("stop_label_offset"s).AsArray()[1]).AsDouble()
			};


			json::Node underlayer_color_node = render_settings_json_map.at("underlayer_color"s);
			if (underlayer_color_node.IsString()) {
				result.underlayer_color = svg::Color(underlayer_color_node.AsString());
			}
			else {
				if (underlayer_color_node.AsArray().size() == 3) {
					result.underlayer_color = svg::Color(MakeRgbFromJsonArray(underlayer_color_node.AsArray()));
				}
				else {
					result.underlayer_color = svg::Color(MakeRgbaFromJsonArray(underlayer_color_node.AsArray()));
				}
			}

			result.underlayer_width = render_settings_json_map.at("underlayer_width"s).AsDouble();

			for (const json::Node& color_node : render_settings_json_map.at("color_palette"s).AsArray()) {
				if (color_node.IsString()) {
					result.color_palette.push_back(color_node.AsString());
				}
				else if (color_node.IsArray()) {
					if (color_node.AsArray().size() == 3) {
						result.color_palette.push_back(MakeRgbFromJsonArray(color_node.AsArray()));
					}
					else {
						result.color_palette.push_back(MakeRgbaFromJsonArray(color_node.AsArray()));
					}
				}
			}
			return result;
		}
		//------------------- Render settings processing end ---------------------//
		
		//------------------- Routing settings processing -----------------------//
		RoutingSettings JsonReader::GetRoutingSettings() const {
			json::Dict routing_settings_map = json_document_.GetRoot().AsMap().at("routing_settings"s).AsMap();
			int bus_wait_time = routing_settings_map.at("bus_wait_time"s).AsInt();
			int bus_velocity = routing_settings_map.at("bus_velocity"s).AsInt();
			return { bus_wait_time, bus_velocity };
		}


		std::string JsonReader::GetSerializationFilename() const {
			return json_document_.GetRoot().AsMap().at("serialization_settings").AsMap().at("file").AsString();
		}

	} // namespace json_handler
} // namespace transport_catalogue