#pragma once

#include "transport_catalogue.h"
#include "graph.h"

#include <transport_catalogue.pb.h>
#include <string>
#include <tuple>

namespace transport_catalogue {
namespace detail {
    transport_catalogue_serialize::Color PackColor(const svg::Color& svg_color);
    svg::Color UnpackColor(const transport_catalogue_serialize::Color& ser_color);

    transport_catalogue_serialize::Stop PackStop(const Stop& stop);
    transport_catalogue_serialize::Bus PackBus(const Bus& bus, const TransportCatalogue& catalogue);
    transport_catalogue_serialize::StopPairDistance PackDistance(const std::pair<const Stop*, const Stop*>& stop_ptr_pair,
                                                                 double distance, const TransportCatalogue& catalogue);

    transport_catalogue_serialize::RenderSettings PackRenderSettings(const map_renderer::detail::RenderSettings& render_settings);
    map_renderer::detail::RenderSettings UnpackRenderSettings(const transport_catalogue_serialize::RenderSettings& ser_render_settings);

    transport_catalogue_serialize::RoutingSettings PackRoutingSettings(const RoutingSettings& routing_settings);
    RoutingSettings UnpackRoutingSettings(const transport_catalogue_serialize::RoutingSettings& ser_routing_settings);

    transport_catalogue_serialize::DirectedWeightedGraph PackGraph(const graph::DirectedWeightedGraph<double>& gr);
    graph::DirectedWeightedGraph<double> UnpackGraph(const transport_catalogue_serialize::DirectedWeightedGraph& ser_gr, size_t vertex_count);
} // namespace detail

    void Serialize(const TransportCatalogue& catalogue, const std::string& filename);
    void Deserialize(const std::string& filename, TransportCatalogue& catalogue);
} // namespace transport_catalogue