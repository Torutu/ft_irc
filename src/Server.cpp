#include "../inc/irc.hpp"

// Default constructor
Server::Server() : cnfg_{}, serverFd_{} {
	// Initialize default values
}
// Parameterized constructor
Server::Server(Config&& configuration) : cnfg_{std::move(configuration)}, serverFd_{} {
	// Initialize with provided values
}
// Destructor
Server::~Server() {
	// Cleanup resources if needed
}

void Server::sendWelcome(int fd) {
	const std::string& nick = clients_[fd].getNick();
	std::string welcome = 
		":localhost 001 " + nick + " :Welcome to the Internet Relay Network\r\n" +
		":localhost 002 " + nick + " :Your host is localhost\r\n" +
		":localhost 003 " + nick + " :This server was created today\r\n" +
		":localhost 004 " + nick + " :localhost 1.0\r\n";
	send(fd, welcome.c_str(), welcome.size(), 0);
}

void Server::checkRegistration(int fd) {
	Client& client = clients_[fd];
	if (!client.getNick().empty() && !client.getUser().empty() && !client.isAuthenticated()) {
		client.setAuthenticated();  // Assuming you want to set them as authenticated
		sendWelcome(fd);
	}
}

void Server::run() {
	std::cout << "Starting server..." << std::endl; // Debug output
	setupServer(); // Set up the server
	mainLoop(); // Start the main loop
}
void Server::setupServer() {
	serverFd_.makeListener(cnfg_.getPort());// Socket wrapper to bind+listen+non-blocking
	// Initialize pollFds_ with server socket
	pollFds_.clear();
	pollFds_.push_back((pollfd){serverFd_.getFd(), POLLIN, 0});
	std::cout << "Server setup complete on port " << cnfg_.getPort() << std::endl;//add IP address
}
void Server::mainLoop() {
	std::cout << "Entering main loop with " << pollFds_.size() << " file descriptors" << std::endl;
	
	while (true) {
		
		int ready = poll(&pollFds_[0], pollFds_.size(), -1);
		if (ready < 0) {
			std::cerr << "poll() error: " << strerror(errno) << std::endl;
			if (errno == EINTR) continue;
			throw std::runtime_error("poll() failed");
		}
		else if (ready == 0) {
			std::cout << "poll() timeout (shouldn't happen with infinite timeout)" << std::endl;
			continue;
		}
		
		// Check all file descriptors, not just server socket
		for (unsigned long i = 0; i < pollFds_.size(); i++) {
			if (pollFds_[i].revents & POLLIN) {
				if (pollFds_.at(i).fd == serverFd_.getFd()) {
					acceptNewConnection();
				}
				else {
					handleClientRead(i);
				}
			}
			if (pollFds_[i].revents & POLLOUT) {
				handleClientWrite(i);
			}
			if (pollFds_[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
				std::cerr << "Error condition on fd " << pollFds_.at(i).fd << std::endl;
				// clients_.erase(pollFds_.at(i).fd); //do we erase the client in this case?
				pollFds_.erase(pollFds_.begin() + i);
				i--; // Adjust index after erase
			}
		}
	}
}
void Server::acceptNewConnection() {
	Socket	clientSock;
	while (serverFd_.accept(clientSock) == false) { //can be while(maxRetries_ private const) or similar
		switch (errno)
		{
			case EINTR:
				continue;
			#if EAGAIN != EWOULDBLOCK
			case EAGAIN:
			#endif
			case EWOULDBLOCK:
			case ECONNABORTED:
			case ENETDOWN:
			case EPROTO:
			case ENOPROTOOPT:
			case EHOSTDOWN:
			case ENONET:
			case EHOSTUNREACH:
			case EOPNOTSUPP:
			case ENETUNREACH:
				return;
			default:
				throw std::system_error(errno, std::generic_category(), "accept() failed");// Fatal errors
		}
	};

	std::cout << "Accepted new connection from " 
			  << inet_ntoa(clientSock.getAddr().sin_addr) << ":"
			  << ntohs(clientSock.getAddr().sin_port) 
			  << " (FD: " << clientSock.getFd() << ")" << std::endl;

	pollFds_.push_back((pollfd){clientSock.getFd(), POLLIN | POLLOUT, 0});

	int	clientSockFd = clientSock.getFd();

	// Initialize new client
	clients_.emplace(clientSock.getFd(), Client(std::move(clientSock)));
	if (clients_.find(clientSockFd) == clients_.end()) {
		std::cout << "Key not found!" << std::endl;
	} else {
		clients_.at(clientSockFd).bufForSending("hellou");
	}
	// Send welcome message
	std::string welcome = "Welcome to ft_irc!\nPlease register with NICK and USER\r\n";
	if (send(clientSockFd, welcome.c_str(), welcome.size(), 0) < 0) {
		std::cerr << "send() error: " << strerror(errno) << std::endl;
	}

	
	// Send initial MOTD (makes irssi happy)
	std::string motd = 
		":localhost 375 * :- Message of the Day -\r\n"
		":localhost 376 * :Another day another slay\r\n";
	send(clientSockFd, motd.c_str(), motd.size(), 0);
}

int Server::getClientFdByNick(const std::string& nick) const {
	for (std::map<int, Client>::const_iterator it = clients_.begin(); it != clients_.end(); ++it) {
		if (it->second.getNick() == nick) {
			return it->first;
		}
	}
	return -1; // Not found
}

std::string Server::getNickByFd(int fd) const {
	std::map<int, Client>::const_iterator it = clients_.find(fd);
	if (it != clients_.end()) {
		return it->second.getNick();
	}
	return ""; // Not found
}

void	Server::handleClientWrite(size_t index) {
	int cliFd = pollFds_.at(index).fd;

	if (clients_[cliFd].sendBuf_.empty()) {
		return;
	}

	ssize_t sent = send(cliFd, clients_[cliFd].sendBuf_.data(), clients_[cliFd].sendBuf_.size(), MSG_NOSIGNAL);
	if (sent > 0) {
		clients_[cliFd].sendBuf_.erase(0, sent);
	} else if (sent == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return;
		} else {
			std::cerr << "FD " << cliFd << " send() error: " << strerror(errno) << std::endl;
		}
	} else if (sent == 0) {
		handleClientError(0, index);
	}
}
