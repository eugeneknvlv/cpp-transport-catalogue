#include "transport_catalogue.h"
#include "request_handler.h"

#include <iostream>
#include <iomanip>
#include <fstream>

using namespace transport_catalogue;
using namespace std::literals;

int main() {

	std::ifstream myfile;
	std::ofstream out_svg_file;
	std::ofstream out_json_file;
	myfile.open("s10_final_opentest_1.json");
	out_svg_file.open("output.svg");
	out_json_file.open("output.json");

	transport_catalogue::TransportCatalogue cat;
	request_handler::RequestHandler request_handler(cat, myfile, out_json_file, out_svg_file);
	request_handler.LoadJsonDataIntoCatalogue();
	request_handler.ProcessJsonStatRequests();
	//request_handler.RenderMap();
	myfile.close();
	out_json_file.close();
	out_svg_file.close();

	system("pause");
	return 0;
}