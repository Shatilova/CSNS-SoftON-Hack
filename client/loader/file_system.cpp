#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include "imgui/imstb_image.h"
#include <direct.h>
#include <unordered_map>
#include <iterator>
#include <sstream>

#include "file_system.h"

#pragma optimize("", off);
namespace internal_fs {
	// files and images are stored in unordered_map
	// because any files/images order is not needed
	std::unordered_map<std::string, easy_packer::File>files;
	std::unordered_map<std::string, Image>images;
	std::string root_dir;

	bool LoadData() {
		easy_packer::unpacker unpack(root_dir + "Loader");
		unpack.set_extension("dat");
		unpack.set_magic_number("SFTN");
		auto result = unpack.process_raw();

		if (result) {
			// transform vector into unordered_map using move-semantics
			std::transform(
				std::make_move_iterator(result->begin()),
				std::make_move_iterator(result->end()),
				std::inserter(files, files.end()),
				[](easy_packer::File && file) {
					return std::make_pair(file.header.name, std::move(file));
				});

			return true;
		}
		// 'process_raw' returned nullopt
		// if something went wrong during processing file
		//
		// todo: should be changed to returning easy_unpacker::get_error()
		else return false;
	}

	bool Initialize() {
		// data-file is stores in ../AppData/Roaming/SoftON/
		root_dir = getenv("APPDATA");
		root_dir += "\\SoftON\\";

		// check if path is exists
		// otherwise create it
		if (!std::filesystem::exists(root_dir))
			std::filesystem::create_directory(root_dir);

		return LoadData();
	}

	const std::string& get_root() {
		return root_dir;
	}

	const easy_packer::File& GetFileByName(const std::string& name) {
		auto el = files.find(name);
		if (el != files.end())
			return el->second;

		std::stringstream ss;
		ss << "File " << name << " not found";
		const std::string tmp_str = ss.str();
		MessageBoxA(0, tmp_str.c_str(), "SoftON Internal Error", 0);
	}

	// original code from 
	// https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples
	//
	// was partially modified by Shatilova
	void GenerateImage(const std::string & name) {
		const easy_packer::File fd = GetFileByName(name);

		Image out;
		int w = 0, h = 0;

		unsigned char* fileRaw = stbi_load_from_memory(&fd.raw[0], fd.header.size, &w, &h, NULL, 4);

		GLuint id;
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, fileRaw);

		stbi_image_free(fileRaw);

		out.name = name.substr(0, name.find_last_of("."));
		out.id = id;
		out.w = w;
		out.h = h;

		images[out.name] = (std::move(out));
	}

	const Image& GetImageByName(const std::string & name) {
		auto el = images.find(name);
		if (el != images.end())
			return el->second;

		std::stringstream ss;
		ss << "Image " << name << " not found";
		const std::string tmp_str = ss.str();
		MessageBoxA(0, tmp_str.c_str(), "SoftON Internal Error", 0);
	}
#pragma optimize("", on);
}