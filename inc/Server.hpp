#pragma once

// System
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <cerrno>
#include <cstring>
#include <csignal>

// Standard Template Library
// System
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <cerrno>
#include <cstring>

// Standard Template Library
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <sstream>
#include <stdexcept>

#include <cctype>

// Project Headers
#include "error.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Socket.hpp"
#include "Config.hpp"
#include "EventHandler.hpp"

//testing
#include <chrono>
#include <thread>
#include <fcntl.h>

class Server;
extern Server	*g_servPtr;

enum IRCState : char {
	IRC_RUNNING   = 0x1,
    IRC_ACCEPTING = 0x2
};

#define REDIRC "\033[1;31m"
#define YELLOWIRC "\033[33m"
#define GREENIRC "\033[1;32m"
#define RESETIRC "\033[0m"

// #define IRC_DEBUG_PRINTS
// #define IRC_POLL_PRINTS
class Server : public EventHandler {
private:
	Config	cfg_;
	Socket	listenSo_;
	volatile sig_atomic_t	state;
	std::map<int, Client>	clients_;
	std::vector<struct pollfd>	pollFds_;
	std::map<std::string, Channel>	channels_;
	int defaultUserCount_ = 0;

	void	updatePollfds();
	void	handleEvents();
	void	addClient(Socket& sock);
	void	rmClient(int rmFd);
	bool	handleMsgs(int fromFd);
	bool	authenticate(Client& newClient, std::string& msg);
	void	processCommand(int fd, const std::string& message);
	
	void	checkRegistration(int fd);
	void	ft_send(int fd, const std::string& message);
	
	void	cmdNick(int fd, const std::string& message);
	void	cmdUser(int fd, const std::string& message);
	void	cmdJoin(int fd, const std::string& message);
	void	cmdPrivMsg(int fd, const std::string& message);
	void	cmdPing(int fd, const std::string& message);
	void	cmdTopic(int fd, const std::string& message);
	void	cmdMode(int fd, const std::string& message);
	void	cmdKick(int sender_fd, const std::vector<std::string>& params);
	void	kickUser(int sender_fd, const std::string& channelName, const std::string& reason, const std::string& targetNick); // Kick user from channel
	
public:
	Server()	= default;
	explicit	Server(Config&& cfg);
	~Server()	= default;
	
	void	run();
	bool	receive() override;
	bool	send() override { return false; }
	void	gracefulShutdown();
	int		getPort() const noexcept {return cfg_.getPort();}
	int		getServerFd() const { return listenSo_.getFd(); }
	int		getClientFdByNick(const std::string& nick) const;
	std::string	getNickByFd(int fd) const;
};
