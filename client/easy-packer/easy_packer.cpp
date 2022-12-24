#include "easy_packer.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iterator>

using namespace std;
namespace fs = std::filesystem;

namespace easy_packer {
	optional<FilesInfo> packer::process() {
		// todo: someday I'll implement multithreading
		/*size_t threads_count = thread::hardware_concurrency();
		if (threads_count == 0) {
			cout << "std::thread::hardware_concurrency() returned 0. The number of threads is set - 4..." << endl;
			threads_count = 4;
		}*/

		FilesInfo result;

		// get files list. continue only if there is files to pack
		auto files_list = this->explore_path();
		if (!files_list.has_value())
			return nullopt;

		// build a header. continue only if there is no files containing
		// whitespaces in them name
		auto header = this->build_header(files_list.value());

		// open output binary file and write header into it
		ofstream bin(
			this->build_fullname().c_str(),
			ios_base::binary
		);
		bin.write(header.c_str(), header.length());

		result.count = files_list.value().size();

		// write all files data into binary-file
		for (auto& [name, size] : files_list.value()) {
			ifstream file_to_pack(name.c_str(), ios_base::binary);

			vector<char> file_data(
				(istreambuf_iterator<char>(file_to_pack)),
				istreambuf_iterator<char>()
			);

			bin.write(
				reinterpret_cast<const char*>(file_data.data()),
				file_data.size()
			);

			file_to_pack.close();

			result.headers.push_back({ move(name), size });
		}

		bin.close();

		return result;
	}

	optional<vector<File>> packer::process_raw() {
		// get files list. continue only if there is files to pack
		auto files_list = this->explore_path();
		if (!files_list.has_value())
			return nullopt;

		vector<File> result;

		// process all files
		for (auto& [name, size] : files_list.value()) {
			ifstream file_to_pack(name.c_str(), ios_base::binary);

			vector<unsigned char> file_data(
				(istreambuf_iterator<char>(file_to_pack)),
				istreambuf_iterator<char>()
			);

			result.push_back(
				{
					{
						move(name),
						size
					},
					move(file_data)
				}
			);

			file_to_pack.close();
		}

		return result;
	}

	optional<vector<FileHeader>> packer::explore_path() {
		vector<FileHeader> files_list;

		// iterates through directory NON-RECURSIVELY!
		for (const auto& entry : fs::directory_iterator(this->path_to)) {
			// only regular files is processed
			if (entry.is_regular_file()) {
				const string filename = entry.path().filename().string();

				if (filename == "Thumbs.db")
					continue;

				if (this->is_executable(filename))
					continue;

				if (this->is_data_file(filename))
					continue;

				files_list.push_back(
					{
						entry.path().filename().string(),
						entry.file_size()
					}
				);
			}
		}

		if (files_list.empty()) {
			this->set_error("There is no files to pack. The directory is empty. Terminating...");
			return nullopt;
		}

		return files_list;
	}

	string packer::build_header(const vector<FileHeader>& files_list) {
		stringstream ss;

		ss.write(this->magic_number.c_str(), this->magic_number.length());

		string files_count(4, ' ');
		files_count = to_string(files_list.size());
		ss.write(files_count.c_str(), 4);

		for (auto& [name, size] : files_list) {
			if (name.find(' ') != name.npos) {
				this->set_error("'" + name + "' contains a space in its name. Skipping...");
				continue;
			}

			ss << name << " " << size << " ";
		}

		return ss.str();
	}

	bool packer::is_executable(const string& filename) const {
		ifstream tmp_file(filename.c_str(), ios_base::binary);

		string file_magic(2, ' ');
		tmp_file.read(file_magic.data(), file_magic.length());
		tmp_file.close();

		if (file_magic == "MZ")
			return true;

		return false;
	}

	bool packer::is_data_file(const string& filename) const {
		ifstream tmp_file(filename.c_str(), ios_base::binary);

		string file_magic(this->magic_number.length(), ' ');
		tmp_file.read(file_magic.data(), file_magic.length());
		tmp_file.close();

		if (file_magic == this->magic_number)
			return true;

		return false;
	}
}