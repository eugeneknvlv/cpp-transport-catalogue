#pragma once
#include "svg.h"
#include "geo.h"
#include "domain.h"

#include <vector>
#include <algorithm>
#include <map>
#include <set>

namespace map_renderer {
    namespace detail {
        struct RenderSettings {
            double width;
            double height;
            double padding;
            double line_width;
            double stop_radius;
            int bus_label_font_size;
            int stop_label_font_size;
            svg::Point stop_label_offset;
            svg::Point bus_label_offset;
            svg::Color underlayer_color;
            double underlayer_width;
            std::vector<svg::Color> color_palette;
        };

        inline const double EPSILON = 1e-6;
        bool IsZero(double value);

        class SphereProjector {
        public:
            SphereProjector() = default;
            // points_begin � points_end ������ ������ � ����� ��������� ��������� geo::Coordinates
            template <typename PointInputIt>
            SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                double max_width, double max_height, double padding)
                : padding_(padding) //
            {
                // ���� ����� ����������� ����� �� ������, ��������� ������
                if (points_begin == points_end) {
                    return;
                }

                // ������� ����� � ����������� � ������������ ��������
                const auto [left_it, right_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
                min_lon_ = left_it->lng;
                const double max_lon = right_it->lng;

                // ������� ����� � ����������� � ������������ �������
                const auto [bottom_it, top_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
                const double min_lat = bottom_it->lat;
                max_lat_ = top_it->lat;

                // ��������� ����������� ��������������� ����� ���������� x
                std::optional<double> width_zoom;
                if (!IsZero(max_lon - min_lon_)) {
                    width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
                }

                // ��������� ����������� ��������������� ����� ���������� y
                std::optional<double> height_zoom;
                if (!IsZero(max_lat_ - min_lat)) {
                    height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
                }

                if (width_zoom && height_zoom) {
                    // ������������ ��������������� �� ������ � ������ ���������,
                    // ���� ����������� �� ���
                    zoom_coeff_ = std::min(*width_zoom, *height_zoom);
                }
                else if (width_zoom) {
                    // ����������� ��������������� �� ������ ���������, ���������� ���
                    zoom_coeff_ = *width_zoom;
                }
                else if (height_zoom) {
                    // ����������� ��������������� �� ������ ���������, ���������� ���
                    zoom_coeff_ = *height_zoom;
                }
            }

            // ���������� ������ � ������� � ���������� ������ SVG-�����������
            svg::Point operator()(geo::Coordinates coords) const {
                return {
                    (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                    (max_lat_ - coords.lat) * zoom_coeff_ + padding_
                };
            }

        private:
            double padding_;
            double min_lon_ = 0;
            double max_lat_ = 0;
            double zoom_coeff_ = 0;
        };

    } // namespace detail

    using BusNameToBusMap = std::map<std::string_view, const transport_catalogue::Bus*>;
    using StopNameToStopMap = std::map<std::string_view, const transport_catalogue::Stop*>;
    class MapRenderer {
    public:
        MapRenderer(const detail::RenderSettings& render_settings,
            const BusNameToBusMap& busname_to_bus_map);
        void SetScalingSettings(std::vector<geo::Coordinates>&& coordinates_to_scale);
        void RenderPolyline(svg::Document& document) const;
        void RenderText(svg::Document& document) const;
        void RenderStopSymbols(svg::Document& document) const;
        void RenderStopNames(svg::Document& document) const;
        svg::Document RenderMap() const;

    private:
        detail::RenderSettings settings_;
        detail::SphereProjector projector_;
        const BusNameToBusMap& busname_to_bus_map_;

        StopNameToStopMap CreateUniqueStopsMap() const;
    };

} // namespace map_renderer