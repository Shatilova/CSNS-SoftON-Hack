#include "easy_packer.h"

#include <iostream>
#include <memory>

using namespace std;

enum class PackerMode : int {
	PACK = 1,
	UNPACK
};
istream& operator >> (istream& is, PackerMode& mode) {
	int tmp;
	is >> tmp;
	mode = static_cast<PackerMode>(tmp);
	return is;
}

PackerMode ProcessPackerMode() {
	cout << "Select the mode:" << endl
		<< "1. Pack" << endl
		<< "2. Unpack" << endl << endl
		<< "> ";
	PackerMode mode;

	cin >> mode;
	while (mode != PackerMode::PACK &&
		mode != PackerMode::UNPACK) {
		cout << "Incorrect mode is entered. Please, try again" << endl
			<< "> ";
		cin >> mode;
	}

	return mode;
}

string ProcessBinName(PackerMode mode) {
	string bin_name;

	cout << endl;
	cout << "Enter an " <<
		(mode == PackerMode::PACK ? "output" : "input") <<
		" file name (without extension):" << endl <<
		"> ";
	cin >> bin_name;

	cout << endl;

	return bin_name;
}

void main() {
	cout << "SoftON Data-File Packer [compiled at " << __DATE__ <<
		" " << __TIME__ << "]" << endl << endl;

	PackerMode mode = ProcessPackerMode();
	string bin_name = ProcessBinName(mode);

	unique_ptr<easy_packer::basic_packer> worker;
	if (mode == PackerMode::PACK)
		worker = make_unique<easy_packer::packer>(bin_name);
	else
		worker = make_unique<easy_packer::unpacker>(bin_name);

	worker->set_magic_number("SFTN");
	worker->set_extension("dat");
	auto result = worker->process();

	if (result.has_value()) {
		cout << "Files count - " << result.value().count << endl;
		cout << "Files:" << endl;
		for (const auto& [name, size] : result.value().headers)
			cout << name << " (" << (size / 1000) << " KB)" << endl;
	}
	else
		cout << "Error: " << worker->get_error() << endl;
}