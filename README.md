[![forthebadge](https://forthebadge.com/images/badges/designed-in-ms-paint.svg)](https://forthebadge.com)
[![forthebadge](https://forthebadge.com/images/badges/reading-6th-grade-level.svg)](https://forthebadge.com)

# Remark of 2022
This repo contains a lot of smelly solutions I've made two years ago. I'd like to completely refactor it but it's too late: the game is dead literally, and I don't have any motivation to loose my time working on this project again.

*The remainder of this document is left as is.*
___
# Project content
## Dependency graph
![dependency graph](https://github.com/Shatilova/CSNS-SoftON-Hack/blob/master/images/dependency%20graph.png)

## 3rd-party data
#### Half-Life 1 SDK
SDK for the GoldSource engine games. Originally intended for HL1, CS 1.6 and associated games. In fact CSN:S is based on CS:CZ engine (modified version of GoldSource), also most of SDK structures was modified by Nexon, but this SDK still may be used with CSN:S. SDK contained in this solution is partially reconstructed (using reverse-engineering) and ready to use with CSN:S. Authors of reconstruction: Shatilova, h4rdee, and JusicP.

Changelog:
* Duplicate files inside different directories deleted.
* Obviously deprecated stuff deleted.
* New functions inside **efx_api_s** and **IPanel** found. Their purpose is unknown.
* New fields found inside **hud_player_info_s**, **cl_entity_s**, **clientdata_s**, **playermove_s**, **weapon_data_s**. Purpose of most of them is unknown.
* **cl_enginefuncs_s** is partially recontructed:
    1. There are a lot of new functions that return pointers to new ingame classes instances. Some of these functions are named, some are not. 
    2. There are client-table functions callbacks. Unlike the corresponding client-table function, callback doesn't take any arguments, and always return 'int'. I've no idea about initial purpose of these callbacks, but probably it was a part of client-table functions protection.
* **enginefuncs_s** is partially reconstructed: it also contains many new functions that return pointers to new ingame classes instances. Some of these functions are named, some are not. 
* **r_studio_interface_s** contains two new funtions:
    1. StudioDrawShopModel - invoked when player is drawn in Inventory/Game Shop tabs. I remember I found there is some mutable data on function stack that can be modified to change player costumes (visual only, of course), but I lost any information about it.
    2. Unknown function that is invoked every game frame. Should be researched. Also we can obtain pointer to 'StudioModelRenderer' instance by offset of that function:
    _*(DWORD*)(*(DWORD*)((DWORD)r_studio_interface_s::Unknown + 0x1))_
* Copy-constructor and copy-assignment operator are defined for the following structures: **cl_enginefuncs_s**, **engine_studio_api_s**, **r_studio_interface_s**, **cl_clientfuncs_s**.
* Added two structures: **client_state_s** and **client_static_s**. Both of them have undergone major changes. **client_state_s** is still changed while big game updates. I wasn't interested in reconstructing it, so I just hidden unknown areas under comments.
* **entity_state_s** has _weaponpainttype_ field that stores information about paint type of current weapon.

The following structures most certainly haven't changed: **con_nprint_s**, **cvar_s**, **dlight_t**, **event_hook_t**, **screenfade_s**, **SCREENINFO_s**, **xcommand_t**, **local_state_s**, **engine_studio_api_s**, **TUserMsg**, **IGameConsole**, **kbutton_s**.

#### GLEW
Cross-platform open-source C/C++ extension loading library.

#### GLFW
Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan application development.

#### ImGui
Bloat-free Graphical User interface for C++ with minimal dependencies.
The library was supplemented with a new widget - HotKey. Code of widget is taken from the Internet. Original author of that widget is unknown.

#### Anti Reverse-Engineering module
Set of techniques to prevent debugging and reverse-engineering your application. 
Original author is Joshua Jackson. Modified by me.

#### VMT Hook
Small piece of code to hook VMT. 
Original author is unknown.
___
## Tools
Simple libraries of my production.

#### Easy Packer
Cross-platform library to process files and glue them into one binary file, and vice versa. 
Features:
* Glue files into binary file
* Load files to memory as raw bytes (includes basic file information like filename and size)
* Unpack files
* Unpack files to memory as raw bytes (includes basic file information like filename and size)

Allows you to change magic number and extension of binary file.

#### Easy HWID
Library to get unique ID of machine. All techniques are taken from the Internet. Using of all included techniques can guarantee strong HWID that's not easy to change unintentionally or specially

#### SoftON Socket
Specific library that implements socket-client with simple encryption of transferred data.
You can read more about client-server relationship below.
___
## Main projects
#### File Packer
Console application to pack files into *.dat binary files. Application must be placed to directory with files to pack.

![file packer](https://github.com/Shatilova/CSNS-SoftON-Hack/blob/master/images/file-packer.png)

#### DLL
The cheat itself. 

![dll](https://github.com/Shatilova/CSNS-SoftON-Hack/blob/master/images/dll.jpg)
![dll2](https://github.com/Shatilova/CSNS-SoftON-Hack/blob/master/images/dll2.jpg)
![dll3](https://github.com/Shatilova/CSNS-SoftON-Hack/blob/master/images/dll3.jpg)

#### Loader
The cheat loader that receives data-files contain images and font used by loader & dll, then receives dll from server, and injects it to the game.

![loader](https://github.com/Shatilova/CSNS-SoftON-Hack/blob/master/images/loader.png)
___
# Client-Server relationship
![server](https://github.com/Shatilova/CSNS-SoftON-Hack/blob/master/images/server.png)

Firstly client-server uses two methods to protect transferred data:
### Encryption
There are two keys 20 characters long: static and dynamic.
* Static key is known to both client and server. It's used by client to encrypt data already encrypted with dynamic key, and used by server to decrypt doubly-encrypted data.
* Dynamic key is generated by client every time client sends data to server. That key is sent to the server together with encrypted data.
### Shuffling
Server generates key that's used for reversible shuffle algorithm. That key is sent to the client for shuffle data back.
___
### Stages
#### Sending data to the server (data is sent to the server is always encrypted, not shuffled)
1. Dynamic key is generated
2. Data to transfer is encrypted with dynamic key
3. Encrypted data is concatenated with dynamic key
4. The resulting data is encrypted with static key
5. Doubly-encrypted data is sent to the server

#### Receiving data from the client
1. Received doubly-encrypted data is decrypted with static key
2. Since key length is static, dynamic key is just separated from data
3. Encrypted with dynamic key data is decrypted with dynamic key
4. Decrypted data is processed

#### Sending data to the client (for example, let's send shuffled data)
1. The random key for shuffling is generated
2. Header of transferring data is built
3. Header is encrypted with dynamic key (that received from client)
4. Header is sent to client
5. The data to send is shuffled with generated random key
6. The data is sent to client

#### Receiving data from the server (for example, let's receive shuffled data)
1. The data header is received and decrypted with dynamic key
2. The random key is extracted from decrypted header
3. The data is received and deshuffled with random key
4. The data is processed
___
# Project structure
* client
    * _anti-re_, _easy-hwid_, _easy-packer_, _softon-socket_ - libraries projects that are used by main projects
    * _dll_, _file-packer_, _loader_ - main projects
    * _assets_ - unpacked files are used by loader/dll
    * _bin_ - built main projects
    * _build_ - object files 
    * _lib_ - built libraries
* images - directory contains images that's used by this README file
* server
    * _*.py_ - server source code
    * _dll.dll_ - the cheat itself
    * _softon_into.txt_ - text that's displayed in a 'Hack Info' field of the loader
    * _softon_cmd_history.log_ - log contains information about any changes of users database
    * _Loader.dat_ - data-file with files used by loader
    * _SoftON.dat_ - data-file with files used by dll
    * _users_info.sqlite_ - users database
___
# Usage guide
#### Using local server
* Build the solution using MSVS19 or higher (C++17 Standard is required)
* Create data-files for loader and dll using **file-packer.exe** from **client/bin/** (files names are _Loader_ and _SoftON_ respectively)
* Place created data-files to **server/**
* Place **dll.dll** from **client/bin/** to **server/**
* Run **server.py** from **server/**
* Run the game
* Run the **loader.exe** from **client/bin/**
* Press _Inject_ once, so look at server console. There is message about requesting for a cheat
* Add youself to the database using _add_ command
* Press _Inject_ again

#### Using VPS
* Change _IP_ and _PORT_ constants inside **main.cpp** of **loader** project, and **main.cpp** of **dll** project, to IP and port of your VPS
* Build the solution using MSVS19 or higher (C++17 Standard is required)
* Change _IP_ and _PORT_ constants inside **server.py** from **server/** to IP and port of your VPS
* Place everything from **server/** to your VPS 
* Create data-files for loader and dll using **file-packer.exe** from **client/bin/** (files names are _Loader_ and _SoftON_ respectively)
* Place created data-files to your VPS
* Place **dll.dll** from **client/bin/** to your VPS
* Run **server.py** from your VPS
* Run the game
* Run the **loader.exe** from **client/bin/**
* Press _Inject_ once, so look at server console. There is message about requesting for a cheat
* Add youself to the database using _add_ command
* Press _Inject_ again

#### Without any server
* Comment _AuthServer_ function invoke inside **main.cpp** of **dll** project
* Build the solution using MSVS19 or higher (C++17 Standard is required)
* Create data-file for the dll using **file-packer.exe** from **client/bin/** (file's name is _SoftON_)
* Place created data-file to **.../AppData/Roaming/SoftON** (make the _SoftON_ folder if it doesn't exists)
* Run the game
* Run any injector you would like to use, and inject **dll.dll** from **client/bin/**
