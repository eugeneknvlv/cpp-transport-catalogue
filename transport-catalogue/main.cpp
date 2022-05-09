#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"

#include <iostream>
#include <iomanip>
#include <fstream>

using namespace transport_catalogue;

int main() {

	//ifstream myfile;
	//ofstream out_file;
	//myfile.open("tsC_case1_input.txt");
	//out_file.open("output.txt");

	transport_catalogue::TransportCatalogue cat;
	input_handler::Reader reader(cat);
	reader.Read(std::cin);

	statistics::StatReader stat_reader(cat, std::cin, std::cout);

	stat_reader.ProcessQueries();
	//myfile.close();
	//out_file.close();

	//string bus = "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye"s;
	//cout << reader.ParseBus(bus).name << endl;

	//reader.ReadFromCin();
	//for (auto stop : cat.stops_) {
	//	cout << stop.name << endl;
	//}

	system("pause");
	return 0;
}