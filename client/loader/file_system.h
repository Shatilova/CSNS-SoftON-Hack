// wrapper for easy_packer library.
// stores files and images,
// and provides access to them via appropriate functions

#pragma once

#include "imgui/imstb_image.h"
#include "easy_packer.h"

namespace internal_fs {
	struct Image {
		unsigned int id;
		int w, h;
		std::string name;
	};

	// i'm not sure that it's good practice to have to invoke special function
	// to make file system is ready to use
	bool Initialize();

	const std::string& get_root();

	const easy_packer::File& GetFileByName(const std::string& name);
	void GenerateImage(const std::string& name);
	const Image& GetImageByName(const std::string& name);
}