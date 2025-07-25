#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <chrono>
#include <poll.h>
#include <cerrno>
#include <cstring>//strerror, strsignal
#include <csignal>
#include <cctype>//toupper
#include <limits.h>//hostnamemax

// Standard Template Library
#include <vector>
#include <set>
#include <unordered_map>
#include <functional>

// Project Headers
#include "error.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Socket.hpp"
#include "Config.hpp"

class Server;
extern Server	*g_servPtr;

typedef struct cmdFunctionParameters {
	std::string					fullMsg;
	std::vector<std::string>	cmdParams;
} t_data;

#define REDIRC "\033[1;31m"
#define YELLOWIRC "\033[33m"
#define GREENIRC "\033[1;32m"
#define RESETIRC "\033[0m"

#define MAX_CLIENTS_IRC 1024

class Server {
	Config		cfg_;
	Socket		listenSo_;
	int			listenSoFd_;
	std::string	ip_;
	std::string	host_;
	bool		accepting_;
	volatile sig_atomic_t	running_;
	std::chrono::seconds	allowedInactivity_;

	std::vector<struct pollfd>	pollFds_;
	std::unordered_map<int, Client>	clients_;
	std::unordered_map<std::string, Channel>	channels_;
	std::unordered_map<std::string, std::function<void(int, const t_data&)>>	cmds_;

	//I/O handling
	void	eventsLoop();
	void	handleEvents(pollfd& pfd);
	void	addClient();
	void	rmClient(int rmFd);
	bool	handleMsgs(int fromFd);
	bool	processAuth(int fromFd, std::string msg);
	void	dispatchCmd(int fd, const std::string& message);

	//commands
	void cmdNick(int fd, const t_data data);
	void cmdUser(int fd, const t_data data);
	void cmdJoin(int fd, const t_data data);
	void cmdPrivMsg(int fd, const t_data data);
	void cmdKick(int fd, t_data data);
	void cmdInvite(int fd, t_data data);
	void cmdTopic(int fd, const t_data data);
	void cmdMode(int fd, const t_data data);
	void cmdPart(int fd, const t_data data);//extra
	void cmdPing(int fd, const t_data& data);//extra
	void cmdWho(int fd, const t_data data);//extra
	
	//utils
	void ft_send(int fd, const std::string& message) { clients_[fd].toSend(message); }
	int getClientFdByNick(const std::string& nick) const;
	void kickUser(int sender_fd, const std::string& channelName, const std::string& reason, const std::string& targetNick);
	void handlePositiveMode(int fd, const std::string& command, const std::string& target, const std::string& modeStr, const std::string& param, Channel& channel);
	void handleNegativeMode(int fd, const std::string& target, const std::string& modeStr, const std::string& param, Channel& channel);
	std::vector<std::string>	tokenize(std::istringstream& cmdParams);
	void errPrint(short revents);

public:
	Server();
	explicit Server(Config&& cfg);
	Server(Server&& other) noexcept;
	Server& operator=(Server&& other) noexcept;
	Server(Server&)				= delete;
	Server& operator=(Server&)	= delete;
	~Server()					= default;

	void	run();
	void	gracefulShutdown();
};
