#include "serialization.h"
#include "map_renderer.h"
#include "svg.h"
#include "graph.h"

#include <fstream>
#include <string_view>
#include <string>

using namespace std;

namespace transport_catalogue {
namespace detail {

transport_catalogue_serialize::Color PackColor(const svg::Color& svg_color) {
    transport_catalogue_serialize::Color ser_color;
    if (holds_alternative<string>(svg_color)) {
        transport_catalogue_serialize::StringColor ser_string_color;
        ser_string_color.set_color(get<string>(svg_color));

        *ser_color.mutable_string_color() = ser_string_color;
        ser_color.clear_rgb();
        ser_color.clear_rgba();
    }
    else if (holds_alternative<svg::Rgb>(svg_color)) {
        transport_catalogue_serialize::Rgb ser_rgb_color;
        svg::Rgb rgb_color = get<svg::Rgb>(svg_color);
        ser_rgb_color.set_r(rgb_color.red);
        ser_rgb_color.set_g(rgb_color.green);
        ser_rgb_color.set_b(rgb_color.blue);

        *ser_color.mutable_rgb() = ser_rgb_color;
        ser_color.clear_string_color();
        ser_color.clear_rgba();
    }
    else {
        transport_catalogue_serialize::Rgba ser_rgba_color;
        svg::Rgba rgba_color = get<svg::Rgba>(svg_color);
        transport_catalogue_serialize::Rgb ser_rgb_color;
        ser_rgb_color.set_r(rgba_color.red);
        ser_rgb_color.set_g(rgba_color.green);
        ser_rgb_color.set_b(rgba_color.blue);
        *ser_rgba_color.mutable_rgb() = ser_rgb_color;
        ser_rgba_color.set_opacity(rgba_color.opacity);

        *ser_color.mutable_rgba() = ser_rgba_color;
        ser_color.clear_string_color();
        ser_color.clear_rgb();
    }
    return ser_color;
}

svg::Color UnpackColor(const transport_catalogue_serialize::Color& ser_color) {
    svg::Color result;
    if (ser_color.has_string_color()) {
        return {ser_color.string_color().color()};
    }
    else if (ser_color.has_rgb()) {
        return {svg::Rgb(ser_color.rgb().r(), ser_color.rgb().g(), ser_color.rgb().b())};
    }
    else {
        return {svg::Rgba(ser_color.rgba().rgb().r(), 
                          ser_color.rgba().rgb().g(), 
                          ser_color.rgba().rgb().b(), 
                          ser_color.rgba().opacity())};
    }
}

transport_catalogue_serialize::Stop PackStop(const Stop& stop) {
    transport_catalogue_serialize::Coordinates ser_coords;
    transport_catalogue_serialize::Stop ser_stop;

    ser_coords.set_lat(stop.coords.lat);
    ser_coords.set_lng(stop.coords.lng);

    ser_stop.set_name(stop.name);
    *ser_stop.mutable_coordinates() = ser_coords;

    return ser_stop;
}

transport_catalogue_serialize::Bus PackBus(const Bus& bus, const TransportCatalogue& catalogue) {
    transport_catalogue_serialize::Bus ser_bus;
    ser_bus.set_name(bus.name);
    ser_bus.set_is_roundtrip(bus.is_roundtrip);

    for (const Stop* stop_ptr : bus.stops) {
        *ser_bus.mutable_stop_index()->Add() = catalogue.GetStopIndex(stop_ptr);
    }

    return ser_bus;
}

transport_catalogue_serialize::StopPairDistance PackDistance(const std::pair<const Stop*, const Stop*>& stop_ptr_pair,
                                                                 double distance, const TransportCatalogue& catalogue)
{
    size_t stop1_idx = catalogue.GetStopIndex(stop_ptr_pair.first);
    size_t stop2_idx = catalogue.GetStopIndex(stop_ptr_pair.second);

    transport_catalogue_serialize::StopPairDistance pair_dist;
    pair_dist.set_stop1_index(stop1_idx);
    pair_dist.set_stop2_index(stop2_idx);
    pair_dist.set_distance(distance);

    return pair_dist;
}

transport_catalogue_serialize::RenderSettings PackRenderSettings(const map_renderer::detail::RenderSettings& render_settings){
    transport_catalogue_serialize::RenderSettings ser_render_settings;
    ser_render_settings.set_width(render_settings.width);
    ser_render_settings.set_height(render_settings.height);
    ser_render_settings.set_padding(render_settings.padding);
    ser_render_settings.set_line_width(render_settings.line_width);
    ser_render_settings.set_stop_radius(render_settings.stop_radius);
    ser_render_settings.set_bus_label_font_size(render_settings.bus_label_font_size);
    ser_render_settings.set_stop_label_font_size(render_settings.stop_label_font_size);

    transport_catalogue_serialize::Point ser_point;
    ser_point.set_x(render_settings.stop_label_offset.x);
    ser_point.set_y(render_settings.stop_label_offset.y);
    *ser_render_settings.mutable_stop_label_offset() = ser_point;

    ser_point.set_x(render_settings.bus_label_offset.x);
    ser_point.set_y(render_settings.bus_label_offset.y);
    *ser_render_settings.mutable_bus_label_offset() = ser_point;

    *ser_render_settings.mutable_underlayer_color() = PackColor(render_settings.underlayer_color);

    ser_render_settings.set_underlayer_width(render_settings.underlayer_width);

    for (const svg::Color& svg_color : render_settings.color_palette) {
        *ser_render_settings.mutable_color_palette()->Add() = PackColor(svg_color);
    }

    return ser_render_settings;
}

transport_catalogue_serialize::RoutingSettings PackRoutingSettings(const RoutingSettings& routing_settings) {
    transport_catalogue_serialize::RoutingSettings ser_routing_settings;
    ser_routing_settings.set_bus_wait_time(routing_settings.bus_wait_time);
    ser_routing_settings.set_bus_velocity(routing_settings.bus_velocity);
    
    return ser_routing_settings;
}

transport_catalogue_serialize::DirectedWeightedGraph PackGraph(const graph::DirectedWeightedGraph<double>& gr) {
    transport_catalogue_serialize::DirectedWeightedGraph ser_gr;
    transport_catalogue_serialize::Edge ser_edge;
    for (const graph::Edge<double>& edge : gr) {
        ser_edge.set_from_id(edge.from);
        ser_edge.set_to_id(edge.to);
        ser_edge.set_span_count(edge.span_count);
        ser_edge.set_bus_name(string(edge.bus_name));
        ser_edge.set_weight(edge.weight);

        *ser_gr.mutable_edges()->Add() = ser_edge;
    }

    return ser_gr;
}

map_renderer::detail::RenderSettings UnpackRenderSettings(const transport_catalogue_serialize::RenderSettings& ser_render_settings) {
    map_renderer::detail::RenderSettings render_settings;
    
    render_settings.width = ser_render_settings.width();
    render_settings.height = ser_render_settings.height();
    render_settings.padding = ser_render_settings.padding();
    render_settings.line_width = ser_render_settings.line_width();
    render_settings.stop_radius = ser_render_settings.stop_radius();
    render_settings.bus_label_font_size = ser_render_settings.bus_label_font_size();
    render_settings.stop_label_font_size = ser_render_settings.stop_label_font_size();
    render_settings.stop_label_offset = {
        ser_render_settings.stop_label_offset().x(),
        ser_render_settings.stop_label_offset().y()
    };
    render_settings.bus_label_offset = {
        ser_render_settings.bus_label_offset().x(),
        ser_render_settings.bus_label_offset().y()
    };

    transport_catalogue_serialize::Color ser_color = ser_render_settings.underlayer_color();
    render_settings.underlayer_color = UnpackColor(ser_color);

    render_settings.underlayer_width = ser_render_settings.underlayer_width();

    size_t color_palette_size = ser_render_settings.color_palette_size();
    for (size_t i = 0; i < color_palette_size; ++i) {
        render_settings.color_palette.push_back(UnpackColor(ser_render_settings.color_palette(i)));
    }

    return render_settings;
}

RoutingSettings UnpackRoutingSettings(const transport_catalogue_serialize::RoutingSettings& ser_routing_settings) {
    transport_catalogue::RoutingSettings routing_settings;
    routing_settings.bus_wait_time = ser_routing_settings.bus_wait_time();
    routing_settings.bus_velocity = ser_routing_settings.bus_velocity();
    
    return routing_settings;
}

graph::DirectedWeightedGraph<double> UnpackGraph(const transport_catalogue_serialize::DirectedWeightedGraph& ser_gr, size_t vertex_count) {
    size_t edges_count = ser_gr.edges_size();
    graph::DirectedWeightedGraph<double> gr(vertex_count);
    for (size_t i = 0; i < edges_count; ++i) {
        const transport_catalogue_serialize::Edge& ser_edge = ser_gr.edges(i);
        gr.AddEdge({
            ser_edge.from_id(),
            ser_edge.to_id(),
            ser_edge.span_count(),
            ser_edge.bus_name(),
            ser_edge.weight()
        });
    }
    
    return gr;
}
} // namespace detail

void Serialize(const TransportCatalogue& catalogue, const string& filename) {
    transport_catalogue_serialize::TransportCatalogue cat_to_serialize;
    const std::deque<Stop>& stops = catalogue.GetStops();
    const std::deque<Bus>& buses = catalogue.GetBuses();
    
    for (const Stop& stop : stops) {
        *cat_to_serialize.mutable_stops()->Add() = detail::PackStop(stop);
    }

    for (const Bus& bus : buses) {
        *cat_to_serialize.mutable_buses()->Add() = detail::PackBus(bus, catalogue);
    }

    for (const auto& [stop_ptr_pair, distance] : catalogue.GetDistances()) {
        *cat_to_serialize.mutable_distances()->Add() = detail::PackDistance(stop_ptr_pair, distance, catalogue);
    }

    const map_renderer::detail::RenderSettings& render_settings = catalogue.GetRenderSettings();
    *cat_to_serialize.mutable_render_settings() = detail::PackRenderSettings(render_settings);


    const transport_catalogue::RoutingSettings& routing_settings = catalogue.GetRoutingSettings();
    *cat_to_serialize.mutable_routing_settings() = detail::PackRoutingSettings(routing_settings);


    const graph::DirectedWeightedGraph<double>& gr = catalogue.GetGraphConstRef();
    *cat_to_serialize.mutable_graph() = detail::PackGraph(gr);

    std::ofstream ofs(filename, ios::binary);
    cat_to_serialize.SerializeToOstream(&ofs);
    ofs.close();
}

void Deserialize(const string& filename, TransportCatalogue& catalogue) {
    transport_catalogue_serialize::TransportCatalogue cat_serialized;
    ifstream ifs(filename, ios::binary);
    cat_serialized.ParseFromIstream(&ifs);
    ifs.close();

    size_t stops_count = cat_serialized.stops_size();
    for (size_t i = 0; i < stops_count; ++i) {
        transport_catalogue_serialize::Stop ser_stop = cat_serialized.stops(i);
        catalogue.AddStop( ser_stop.name(), {ser_stop.coordinates().lat(), ser_stop.coordinates().lng()} );
    }

    size_t buses_count = cat_serialized.buses_size();
    for (size_t i = 0; i < buses_count; ++i) {
        transport_catalogue_serialize::Bus ser_bus = cat_serialized.buses(i);
        std::vector<std::string> stops;
        size_t stops_count = ser_bus.stop_index_size();
        for (size_t j = 0; j < stops_count; ++j) {
            stops.push_back(catalogue.GetStopnameByIndex(ser_bus.stop_index(j)));
        }
        catalogue.AddBus(ser_bus.name(), stops, ser_bus.is_roundtrip());
    }

    size_t distances_count = cat_serialized.distances_size();
    for (size_t i = 0; i < distances_count; ++i) {
        transport_catalogue_serialize::StopPairDistance stop_pair_distance = cat_serialized.distances(i);
        std::string stop1_name = catalogue.GetStopnameByIndex(stop_pair_distance.stop1_index());
        std::string stop2_name = catalogue.GetStopnameByIndex(stop_pair_distance.stop2_index());
        double distance = stop_pair_distance.distance();

        catalogue.SetDistance(stop1_name, stop2_name, distance);
    }

    transport_catalogue_serialize::RenderSettings ser_render_settings = cat_serialized.render_settings();
    catalogue.SetRenderSettings(detail::UnpackRenderSettings(ser_render_settings));

    transport_catalogue_serialize::RoutingSettings ser_routing_settings = cat_serialized.routing_settings();
    catalogue.SetRoutingSettings(detail::UnpackRoutingSettings(ser_routing_settings));

    
    transport_catalogue_serialize::DirectedWeightedGraph ser_graph = cat_serialized.graph();
    catalogue.SetGraph(detail::UnpackGraph(ser_graph, catalogue.GetStops().size()));
}

} // namespace transport_catalogue