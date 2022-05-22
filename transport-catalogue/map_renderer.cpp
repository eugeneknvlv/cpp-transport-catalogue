#include "map_renderer.h"

using namespace std::literals;

namespace map_renderer {
	namespace detail {
		bool IsZero(double value) {
			return std::abs(value) < EPSILON;
		}

	}

	MapRenderer::MapRenderer(const detail::RenderSettings& render_settings,
		const BusNameToBusMap& busname_to_bus_map)
		: settings_(render_settings)
		, busname_to_bus_map_(busname_to_bus_map)
	{}

	StopNameToStopMap MapRenderer::CreateUniqueStopsMap() const {
		StopNameToStopMap result;
		for (const auto& [bus_name, bus_ptr] : busname_to_bus_map_) {
			for (const transport_catalogue::Stop* stop : bus_ptr->stops) {
				result.insert({ stop->name, stop });
			}
		}
		return result;
	}

	void MapRenderer::SetScalingSettings(std::vector<geo::Coordinates>&& coordinates_to_scale) {
		projector_ = detail::SphereProjector(coordinates_to_scale.begin(), coordinates_to_scale.end(),
			settings_.width, settings_.height, settings_.padding
		);
	}

	void MapRenderer::RenderPolyline(svg::Document& document) const {
		int color_selector = -1;
		for (const auto& [bus_name, bus_ptr] : busname_to_bus_map_) {
			if (!(bus_ptr->stops).empty()) {
				color_selector++;
			}
			else {
				continue;
			}
			svg::Polyline polyline;
			polyline.SetFillColor("none"s)
				.SetStrokeColor(settings_.color_palette[color_selector % settings_.color_palette.size()])
				.SetStrokeWidth(settings_.line_width)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			for (int i = 0; i < (bus_ptr->stops).size(); i++) {
				const transport_catalogue::Stop* stop = bus_ptr->stops[i];
				polyline.AddPoint(projector_(stop->coords));
			}
			if (!bus_ptr->is_roundtrip) {
				for (int i = bus_ptr->stops.size() - 2; i >= 0; i--) {
					const transport_catalogue::Stop* stop = bus_ptr->stops[i];
					polyline.AddPoint(projector_(stop->coords));
				}
			}
			document.Add(polyline);
		}
	}

	void MapRenderer::RenderText(svg::Document& document) const {
		int color_selector = -1;
		for (const auto& [bus_name, bus_ptr] : busname_to_bus_map_) {
			if (!(bus_ptr->stops).empty()) {
				color_selector++;
			}
			else {
				continue;
			}

			svg::Text general;
			general.SetPosition(projector_(bus_ptr->stops[0]->coords))
				.SetOffset(settings_.bus_label_offset)
				.SetFontSize(settings_.bus_label_font_size)
				.SetFontFamily("Verdana"s)
				.SetFontWeight("bold"s)
				.SetData(std::string(bus_name));

			svg::Text substrate_general = general;
			svg::Text first_stop_substrate = substrate_general.SetFillColor(settings_.underlayer_color)
				.SetStrokeColor(settings_.underlayer_color)
				.SetStrokeWidth(settings_.underlayer_width)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			svg::Text first_stop_text_general = general;
			svg::Text first_stop_text = first_stop_text_general.SetFillColor(settings_.color_palette[color_selector % settings_.color_palette.size()]);
			document.Add(first_stop_substrate);
			document.Add(first_stop_text);
			if (!(bus_ptr->is_roundtrip) && bus_ptr->stops[0] != bus_ptr->stops[(bus_ptr->stops).size() - 1]) {
				svg::Text last_stop_substrate = first_stop_substrate.SetPosition(projector_(bus_ptr->stops[bus_ptr->stops.size() - 1]->coords));
				svg::Text last_stop_text = first_stop_text.SetPosition(projector_(bus_ptr->stops[bus_ptr->stops.size() - 1]->coords));
				document.Add(last_stop_substrate);
				document.Add(last_stop_text);
			}
		}
	}

	void MapRenderer::RenderStopSymbols(svg::Document& document) const {
		StopNameToStopMap stopname_to_stop_map = CreateUniqueStopsMap();
		for (const auto& [stop_name_sv, stop_ptr] : stopname_to_stop_map) {
			svg::Circle circle;
			circle.SetCenter(projector_(stop_ptr->coords))
				.SetRadius(settings_.stop_radius)
				.SetFillColor("white"s);
			document.Add(circle);
		}
	}

	void MapRenderer::RenderStopNames(svg::Document& document) const {
		StopNameToStopMap stopname_to_stop_map = CreateUniqueStopsMap();
		for (const auto& [stop_name_sv, stop_ptr] : stopname_to_stop_map) {
			svg::Text general;
			general.SetPosition(projector_(stop_ptr->coords))
				.SetOffset(settings_.stop_label_offset)
				.SetFontSize(settings_.stop_label_font_size)
				.SetFontFamily("Verdana"s)
				.SetData(std::string(stop_name_sv));

			svg::Text substrate_general = general;
			svg::Text substrate = substrate_general.SetFillColor(settings_.underlayer_color)
				.SetStrokeColor(settings_.underlayer_color)
				.SetStrokeWidth(settings_.underlayer_width)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

			svg::Text text_general = general;
			svg::Text text = text_general.SetFillColor("black"s);
			document.Add(substrate);
			document.Add(text);
		}
	}

	svg::Document MapRenderer::RenderMap() const {
		svg::Document result;
		RenderPolyline(result);
		RenderText(result);
		RenderStopSymbols(result);
		RenderStopNames(result);
		return result;
	}
}