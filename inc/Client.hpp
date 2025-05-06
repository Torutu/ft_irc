#pragma once

#include <iostream>
#include <string>
#include <map>
#include "Socket.hpp"

//add send/recv with persisting buffers to client
class Client {
	private:
		Socket	so_;
		std::string nick; // Client's nickname
		std::string user; // Client's username
		std::map<std::string, bool> joinedChannels; // Set of channels the client has joined
		bool authenticated; // Authentication status
		bool nickReceived; // Nickname received status
		bool userReceived; // Username received status
		bool passReceived; // Password received status
		bool modeReceived; // Mode received status
		bool whois; // Whois status

	public:
		Client()								= default;
		Client(Socket&& so); // only constructor; ties the client instance to a socket
		// Client(std::string nick, std::string user, int fd);
		Client(const Client& other)				= delete;
		Client& operator=(const Client& other)	= delete;
		Client(Client&& other) noexcept; //Move constructor
		Client&	operator=(Client&& other) noexcept; //Move assignment
		~Client()								= default;

		std::string getNick() const;
		std::string getUser() const;

		bool hasReceivedNick() const;
		bool hasReceivedUser() const;
		bool hasReceivedPass() const;
		bool hasReceivedMode() const;
		bool isAuthenticated() const;
		int getFd() const;
		const std::map<std::string, bool>& getJoinedChannels() const;

		void setNick(const std::string& nick);
		void setUser(const std::string& user);
		void setNickReceived();
		void setUserReceived();
		void setPassReceived();
		void setModeReceived();
		void setAuthenticated();

		void joinChannel(const std::string& channel, bool is_operator);
		void leaveChannel(const std::string& channel);// Leave a channel

		bool isInChannel(const std::string& channel); // Check if client is in a channel
		bool isOperator(const std::string& channel); // Check if client is an operator in a channel

		bool getOperator(const std::string& channel);
		void setOperator(const std::string& channel, bool is_operator); // Set operator status
	};
