#include "../inc/irc.hpp"
#include "../inc/Socket.hpp"
//irc server
// ./irc <port> <password>
int main(int argc, char* argv[]) {
	try {
	Server server; // Create a Server object
	if (argCheck(argc, argv, server) == 1)//check if the user has provided the correct number of arguments
		return 1;
	Socket sock;
	(void) sock;
	server.run();
	//print server information
	std::cout << "Server Port: " << server.getPort() << std::endl;
	std::cout << "Server Password: " << server.getPassword() << std::endl;
	}catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
