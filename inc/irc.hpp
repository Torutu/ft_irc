#ifndef IRC_HPP
#define IRC_HPP

#include <filesystem> // good practice file system methods (originally from ACE framework implemented in C++17)

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cctype>
#include <stdexcept>
#include <sys/socket.h> // For socket functions
#include <netinet/in.h> 
#include <unistd.h> // For close function
#include <cstring> 
#include <cerrno> 
#include <stdexcept> // For std::runtime_error
#include <poll.h>
#include <arpa/inet.h> // For inet_ntop
#include <sstream> // For std::cerr
#include <fcntl.h> // For fcntl

class Server {
public:
	// setters
	void setPort(const int port) { port_ = port; }
	void setPassword(const std::string& password) { password_ = password; }
	void setServerFd(int fd) { serverFd_ = fd; }
	// getters
	int getPort() const { return port_; }
	const std::string& getPassword() const { return password_; }
	int getServerFd() const { return serverFd_; }
	void run(); // Start the server
	Server();// Default constructor
	Server(const int port, const std::string& password); // Parameterized constructor
	~Server();
private:
	void setupServer(); // Set up the server
	void mainLoop(); // Main loop for handling connections
	void acceptNewConnection(); // Accept new connections
	void handleClient(size_t index); // Handle client communication
	void checkRegistration(int fd); // Check if client is registered
	void sendWelcome(int fd); // Send welcome message to client

	int port_;
	std::string password_;
	int serverFd_;
	std::vector<int> clientFds_; // Vector of client file descriptors
	std::vector<struct pollfd> pollFds_; // Vector of poll file descriptors for clients
	struct Client {
		std::string nick;
		std::string user;
		bool registered;
	};
	std::map<int, Client> clients_; // Map of client file descriptors to Client objects
};
	
	int argCheck(int argc, char* argv[], Server& server); // Take Server reference as parameter
	bool isValidPort(const std::string& port);
	bool isValidPassword(const std::string& password);

#endif // IRC_HPP