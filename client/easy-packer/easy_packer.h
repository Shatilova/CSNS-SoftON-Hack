// library to process files and/or pack them into binary file
//
// it's quite primitive tool right now. namely:
// 1) operations are executed in a single thread
// 2) packer application should be placed into directory with files to pack
// since there is no fully support of customized directories
// 3) exe-files are ignored (to avoid processing packer application)
// 4) not all possible errors and issues are handled
// 5) folders are ignored
// 6) files containing whitespaces isn't allowed
// 7) binary-file is not encrypted. it may be quite unsafe

#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <optional>

namespace easy_packer {
	// basic information about raw file: name and size
	struct FileHeader {
		std::string name;
		uintmax_t size = 0;
	};

	// basic information about packed file: count of files, and headers of all files
	struct FilesInfo {
		uintmax_t count = 0;
		std::vector<FileHeader> headers;
	};

	// file header and it's raw data
	struct File {
		FileHeader header;
		std::vector<unsigned char> raw;
	};

	// base class for packer and unpacker
	class basic_packer {
	public:
		// about magic number:
		// https://en.wikipedia.org/wiki/Magic_number_(programming)
		void set_magic_number(const std::string& magic_) { this->magic_number = magic_; }
		void set_magic_number(const std::string&& magic_) { this->magic_number = std::move(magic_); }

		void set_extension(const std::string& ext_) { this->extension = ext_; }
		void set_extension(const std::string&& ext_) { this->extension = std::move(ext_); }

		virtual std::optional<FilesInfo> process() = 0;
		virtual std::optional<std::vector<File>> process_raw() = 0;

		// errors occured while the packer/unpacker was running
		// are saved to the buffer and may be get from
		const std::string& get_error() const { return this->error; }

	protected:
		std::string bin_name;
		std::string magic_number = "DAT";
		std::string error = "NO ERROR";
		std::string extension = "dat";

		// make fullname from filename and its extension
		std::string build_fullname() const { return this->bin_name + "." + this->extension; }

		// errors occured while packer/unpacker was running can be saved to buffer
		void set_error(const std::string& error_) { this->error = error_; }
		void set_error(const std::string&& error_) { this->error = std::move(error_); }
	};

	class packer : public basic_packer {
	public:
		// 'bin_name' is a name of output binary-file.
		// 'path_to' is path to files to pack. this argument is optional.
		// if unspecified then binary-file is created 
		// from files in directory with current program
		packer(
			const std::string& bin_name,
			const std::string& path_to = std::filesystem::current_path().string()
		) :
			path_to(path_to)
		{
			this->bin_name = bin_name;
		}

		// takes files and glue them into binary-file
		std::optional<FilesInfo> process() override;

		// takes files and interpret them as vector of headers and raw bytes
		std::optional<std::vector<File>> process_raw() override;

	private:
		std::string path_to;

		// collect files list to pack
		std::optional<std::vector<FileHeader>> explore_path();

		// create header of binary-file using files list
		std::string build_header(const std::vector<FileHeader>& files_list);

		// check if the file that processing by 'explore_path' is an executable file
		bool is_executable(const std::string& filename) const;

		// check if file that processing by 'explore_path' is a binary file
		// with the same magic number as current class instance 
		bool is_data_file(const std::string& filename) const;
	};

	class unpacker : public basic_packer {
	public:
		unpacker(const std::string& bin_name) {
			this->bin_name = bin_name;
		}

		// takes binary-file and unpack it into folder named
		// "%bin_name%.%extension%_unpacked"
		std::optional<FilesInfo> process() override;

		// takes binary-file, and interpret it as vector of headers and raw bytes
		std::optional<std::vector<File>> process_raw() override;
	};
}