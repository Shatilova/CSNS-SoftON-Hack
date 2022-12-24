// code-smell socket-wrapper using WinSock-s

#pragma once

#include <winsock2.h>

#include <iostream>
#include <string>
#include <optional>

class CSocket {
public:
	CSocket();
	~CSocket();

	// takes ip:port, tries to connect
	// return nullopt if connection established successfully
	std::optional<std::string> connect(const std::string& ip, const std::string& port);

	void disconnect();
	void send(std::string&& msg);
	unsigned char* listen();

	// todo: storing last_listen_size should be rethinked, 
	//		 and this method should be removed
	int get_last_listen_size() { return this->last_size; }

private:
	// in fact deprecated and useless enum
	// should indicate socket state,
	// but generally it's more convenient to somehow display
	// messages about errors when they are occured,
	// or even throw an exception
	enum class SocketState {
		FAILED,
		READY,
		CONNECTED
	};

	// types of received sockets
	// different types are processed differently by 'listen' method
	enum class HeaderType {
		MESSAGE,
		RAW,
		RESPONCE,
		COMMAND
	};

	std::string str_xor(const std::string& msg, const std::string& key);
	const std::string& get_key() { return this->key; }

	// generate new 20-symbols-size key to encrypt data to send.
	// 20 is an arbitrary key length
	void update_key();

	SOCKET socket;
	SocketState state;
	std::string key;
	int last_size;
};