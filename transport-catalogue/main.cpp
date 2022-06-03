#include "transport_catalogue.h"
#include "request_handler.h"

#include <iostream>
#include <iomanip>
#include <fstream>

using namespace transport_catalogue;
using namespace std::literals;

int main() {

	std::ifstream myfile;
	std::ofstream out_file;
	myfile.open("test.json");
	out_file.open("output.json");

	transport_catalogue::TransportCatalogue cat;
	request_handler::RequestHandler request_handler(cat, myfile, out_file, std::cout);
	request_handler.LoadJsonDataIntoCatalogue();
	request_handler.ProcessJsonStatRequests();
	/*request_handler.RenderMap(std::cout);*/

	system("pause");
	return 0;
}