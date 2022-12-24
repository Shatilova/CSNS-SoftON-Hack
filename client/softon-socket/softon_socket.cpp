#define _CRT_SECURE_NO_WARNINGS

#include "softon_socket.h"

#include <ws2tcpip.h>
#include <random>
#include <ctime>

#include "array_shuffle.h"

// static key is used to reencryption data
const std::string& STATIC_KEY = "i9jpODi2hql2EpaiDr4WjF5s7Cw96QcDdGNRfkvo";

CSocket::CSocket() {
	WSADATA wsaData;
	int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != 0)
		this->state = SocketState::FAILED;
	else {
		this->socket = INVALID_SOCKET;
		this->state = SocketState::READY;
	}
}

CSocket::~CSocket() {
	shutdown(this->socket, SD_SEND);
	closesocket(this->socket);
	WSACleanup();
}

std::optional<std::string> CSocket::connect(const std::string& ip, const std::string& port) {
	if (this->state != SocketState::READY) {
		WSACleanup();
		return "Socket state is  - " +
			std::to_string(static_cast<int>(this->state)) +
			". Connection refused";
	}

	int result;
	struct addrinfo* addr = NULL, * ptr = NULL, hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	result = getaddrinfo(ip.c_str(), port.c_str(), &hints, &addr);
	if (result != 0) {
		WSACleanup();
		return "Server not found. Result - " + std::to_string(result);
	}

	for (ptr = addr; ptr != NULL; ptr = ptr->ai_next) {
		this->socket = ::socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (this->socket == INVALID_SOCKET) {
			WSACleanup();
			return "Socket failed with error " + std::to_string(WSAGetLastError());
		}

		result = ::connect(this->socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (result == SOCKET_ERROR) {
			closesocket(this->socket);
			this->socket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(addr);

	if (this->socket == INVALID_SOCKET) {
		WSACleanup();
		return "Unable to connect to the server!\nPlease try again later, or check hack news on vk.com/general_hack";
	}

	this->state = SocketState::CONNECTED;

	return std::nullopt;
}

void CSocket::disconnect() {
	int res = shutdown(this->socket, SD_SEND);
	closesocket(this->socket);
}

void CSocket::send(std::string&& msg) {
	this->update_key();

	// finilizing message with " 0" is needed
	// to make message parsing more convenient
	// todo: i could be wrong, and this "postfix" might be
	//		 totally useless. should be rethinked
	std::string out = std::move(msg) + " 0";

	// encrypt message with dynamic key 
	out = this->str_xor(out, this->key);

	// concatenate encrypted message with
	// encrypted dynamic key with static key
	out += this->str_xor(this->key, STATIC_KEY);

	int res = ::send(this->socket, out.c_str(), out.size(), 0);

	if (res == SOCKET_ERROR) {
		closesocket(this->socket);
		WSACleanup();
		return;
	}
}

unsigned char* CSocket::listen() {
	const int BUFLEN = 1024;

	char* recvbuf = new char[BUFLEN];
	int res = 0;
	res = recv(this->socket, recvbuf, BUFLEN, 0);

	if (res <= 0)
		return NULL;

	// first 256 bytes of every message is a header
	// that contains different data depends on message type
	std::string header = this->str_xor(std::string(recvbuf, 256), this->key);
	unsigned char* internal_buf = nullptr;

	HeaderType type;
	size_t delim_pos = header.find("=");
	std::string type_str = header.substr(0, delim_pos);

	// check for message type
	if (type_str == "msg") type = HeaderType::MESSAGE;
	else if (type_str == "dat") type = HeaderType::RAW;
	else if (type_str == "rsp") type = HeaderType::RESPONCE;
	else if (type_str == "cmd") type = HeaderType::COMMAND;

	// peek header excluding message type
	header = header.substr(delim_pos + 1);

	size_t last = 0, next = 0;
	std::vector<std::string>words;
	// split header data by commas
	while ((next = header.find(",", last)) != std::string::npos) {
		words.push_back(header.substr(last, next - last));
		last = next + 1;
	}

	// COMMAND is a message type that contains a command mnenonic 
	// that is interpreted by a client
	//
	// in that case there is the only command-handler: for 'terminate' command
	if (type == HeaderType::COMMAND) {
		if (words[0].find("terminate") == 0)
			TerminateProcess(GetCurrentProcess(), 0);

		return nullptr;
	}

	// RESPONCE is a message type that contains message from server to a client
	// for example: Your license is invalid
	if (type == HeaderType::RESPONCE)
		return (unsigned char*)words[0].c_str();

	// words[1] contains a data size of MESSAGE and RAW messages
	// we need to allocate memory to store that data
	//
	// todo: low-level memory allocation should be avoided of!
	int internal_sz = std::stoi(words[1]), internal_queue = 0;
	internal_buf = new unsigned char[internal_sz];
	this->last_size = internal_sz;

	// read from socket while it isn't out of range sent data
	while (internal_queue < internal_sz) {
		res = recv(this->socket, recvbuf, BUFLEN, 0);
		if (res <= 0)
			break;

		// MESSAGE is a text-file
		//
		// ambigious name. should be renamed
		if (type == HeaderType::MESSAGE) {
			memcpy(&internal_buf[internal_queue], this->str_xor(std::string(recvbuf, res), this->key).c_str(), res);
			internal_queue += res;
		}
		// RAW is a binary-file
		else if (type == HeaderType::RAW) {
			memcpy(&internal_buf[internal_queue], recvbuf, res);
			internal_queue += res;
		}
	}

	// text-files sent with MESSAGE aren't encrypted
	if (type == HeaderType::MESSAGE) {
		std::string tmp = reinterpret_cast<const char*>(internal_buf);
		return (unsigned char*)(tmp.substr(0, tmp.find("#")).c_str());
	}
	// binary-files sent with RAW are shuffled
	// and should be deshuffled before use.
	// the key for deshuffle is stored in words[2]
	else if (type == HeaderType::RAW)
		internal_buf = ArratDeshuffle<unsigned char>(internal_buf, internal_sz, std::stoi(words[2]));

	return internal_buf;
}

// simple XOR implementation
std::string CSocket::str_xor(const std::string & msg, const std::string & key) {
	std::string out;
	for (size_t i = 0, j = 0; i < msg.size(); ++i, ++j) {
		if (j >= key.size())
			j = 0;

		out += msg[i] ^ key[j];
	}

	return out;
}

void CSocket::update_key() {
	const int LENGTH = 20;
	const std::string alphabet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	std::uniform_int_distribution<signed>s(0, 62);
	std::default_random_engine e(time(0));

	this->key.clear();
	for (int i = 0; i < LENGTH; i++)
		this->key += alphabet[s(e)];
}