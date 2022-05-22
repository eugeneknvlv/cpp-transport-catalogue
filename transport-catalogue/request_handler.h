#pragma once
#include "transport_catalogue.h"
#include "json_reader.h"

#include <vector>
#include <map>

namespace transport_catalogue {
	namespace request_handler {
		class RequestHandler {
		public:
			RequestHandler(TransportCatalogue& catalogue, std::istream& json_input, std::ostream& json_output, std::ostream& svg_output);

			void LoadJsonDataIntoCatalogue();
			void ProcessJsonStatRequests() const;

			void RenderMap() const;

		private:
			TransportCatalogue& catalogue_;
			json_handler::JsonReader json_reader_;
			std::ostream& svg_output_;
		};
	} // namespace request_handler
} // namespace transport_catalogue