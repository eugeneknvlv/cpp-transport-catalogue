#include "request_handler.h"
#include "map_renderer.h"
#include <set>
#include <string_view>

namespace transport_catalogue {
	namespace request_handler {

		RequestHandler::RequestHandler(TransportCatalogue& catalogue, std::istream& json_input, std::ostream& json_output, std::ostream& svg_output)
			: catalogue_(catalogue)
			, json_reader_(catalogue, json_input, json_output)
			, svg_output_(svg_output)
		{}

		void RequestHandler::LoadJsonDataIntoCatalogue() {
			json_reader_.LoadJSON();
			json_reader_.ProcessBaseRequests();
		}

		void RequestHandler::ProcessJsonStatRequests() const {
			json_reader_.ProcessStatRequests();
		}



		void RequestHandler::RenderMap() const {
			map_renderer::MapRenderer renderer(json_reader_.GetRenderSettings(),
				catalogue_.GetBusnameToBusMap());
			renderer.SetScalingSettings(catalogue_.GetEveryBusPointCoordinates());
			svg::Document result = renderer.RenderMap();
			result.Render(svg_output_);
		}

	} // namespace request_handler
} // namespace transport_catalogue