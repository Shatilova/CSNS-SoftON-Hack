#include "easy_packer.h"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

namespace easy_packer {
	std::optional<FilesInfo> unpacker::process() {
		FilesInfo result;

		// open input binary-file
		ifstream bin(
			this->build_fullname().c_str(),
			ios_base::binary
		);

		// read magic number from binary-file and compare it with
		// sought magic number
		string file_magic(this->magic_number.length(), ' ');
		bin.read(file_magic.data(), file_magic.length());
		if (file_magic != this->magic_number) {
			this->set_error("Data file was expected");
			return nullopt;
		}

		// create directory to place unpacked files
		fs::path work_dir = this->build_fullname() + "_unpacked/";
		fs::create_directory(work_dir);

		// read files count from binary-file
		string files_count(4, ' ');
		bin.read(files_count.data(), 4);

		// read the whole data of binary-file, and close it
		stringstream ss;
		ss << bin.rdbuf();
		bin.close();

		// process header
		vector<FileHeader> files_list;
		size_t n = stoi(files_count);
		result.count = n;
		while (n--) {
			string name;
			uintmax_t size;

			ss >> name >> size;
			files_list.push_back(
				{
					name,
					size
				}
			);

			result.headers.push_back(
				{
					move(name),
					size
				}
			);
		}

		// ignore whitespace after header
		ss.ignore(1);

		// process files bytes based on header
		for (auto& [name, size] : files_list) {
			ofstream unpacked_file(string(work_dir.string() + name).c_str(), ios_base::binary);

			string raw(size, ' ');
			ss.read(raw.data(), size);
			unpacked_file.write(raw.c_str(), raw.size());

			unpacked_file.close();
		}

		return result;
	}

	// todo: refactor, remove code duplication
	optional<vector<File>> unpacker::process_raw() {
		// open input binary-file
		ifstream bin(
			this->build_fullname().c_str(),
			ios_base::binary
		);

		// read magic number from binary-file and compare it with
		// sought magic number
		string file_magic(this->magic_number.length(), ' ');
		bin.read(file_magic.data(), file_magic.length());
		if (file_magic != this->magic_number) {
			this->set_error("Data file was expected");
			return nullopt;
		}

		// read files count from binary-file
		string files_count(4, ' ');
		bin.read(files_count.data(), 4);

		// read the whole data of binary-file, and close it
		stringstream ss;
		ss << bin.rdbuf();
		bin.close();

		// process header
		vector<FileHeader> files_list;
		size_t n = stoi(files_count);
		while (n--) {
			string name;
			uintmax_t size;

			ss >> name >> size;
			files_list.push_back(
				{
					name,
					size
				}
			);
		}

		// ignore whitespace after header
		ss.ignore(1);

		// process files bytes based on header
		vector<File> result;
		for (auto& [name, size] : files_list) {
			string raw(size, ' ');
			ss.read(raw.data(), size);

			// todo: move-semantics
			result.push_back(
				{
					{
						move(name),
						size
					},
					{
						make_move_iterator(raw.begin()),
						make_move_iterator(raw.end())
					}
				}
			);
		}

		return result;
	}
}