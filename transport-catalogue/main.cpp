#include "transport_catalogue.h"
// #include "request_handler.h"
#include "json_reader.h"
#include "serialization.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

using namespace transport_catalogue;
using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, const char** argv) {
	if (argc != 2) {
        PrintUsage();
        return 1;
    }

	const std::string_view mode(argv[1]);

	if (mode == "make_base"s) {
		transport_catalogue::TransportCatalogue catalogue;
		// request_handler::RequestHandler request_handler(catalogue);
		json_handler::JsonReader json_reader(catalogue);
		std::ifstream input("/home/eugene/ya_pract/cpp/cpp-transport-catalogue/tests/s14_3_opentest_3_make_base.json");
		json_reader.LoadJSON(input);
		json_reader.ProcessBaseRequests();
		// request_handler.LoadJsonDocument(input);
		// request_handler.LoadJsonDataIntoCatalogue();
		const size_t vertex_count = catalogue.GetGraphConstRef().GetVertexCount();
		std::string filename = json_reader.GetSerializationFilename();
		Serialize(catalogue, filename);
	}
	else if (mode == "process_requests"s) {
		transport_catalogue::TransportCatalogue catalogue;
		json_handler::JsonReader json_reader(catalogue);
		std::ifstream input("/home/eugene/ya_pract/cpp/cpp-transport-catalogue/tests/s14_3_opentest_3_process_requests.json");
		json_reader.LoadJSON(input);

		std::string filename = json_reader.GetSerializationFilename();
		Deserialize(filename, catalogue);

		std::ofstream output("/home/eugene/ya_pract/cpp/cpp-transport-catalogue/test_answers/1.json");
		output << std::setprecision(6) << std::fixed;
		const size_t vertex_count = catalogue.GetGraphConstRef().GetVertexCount();
		json_reader.ProcessStatRequests(output);
	}
	else {
		PrintUsage();
        return 1;
	}
	// std::ifstream myfile("/home/eugene/ya_pract/cpp/cpp-transport-catalogue/transport-catalogue/input.json");
	// std::ofstream out_file("/home/eugene/ya_pract/cpp/cpp-transport-catalogue/transport-catalogue/output.json");

	// transport_catalogue::TransportCatalogue cat;
	// request_handler::RequestHandler request_handler(cat);
	// request_handler.LoadJsonDataIntoCatalogue(myfile);
	// request_handler.ProcessJsonStatRequests(out_file);
	/*request_handler.RenderMap(std::cout);*/
	return 0;
}