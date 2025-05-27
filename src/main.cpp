#include "../inc/Server.hpp"
#include "../inc/Reactor.hpp"

namespace {

	void	sigShutdown(int signal) {
		std::cerr << "\nSignal caught - signum: " << signal << std::endl;
		if (g_servPtr) {
			g_servPtr->gracefulShutdown();
		}
	}
}

int main(int argc, char* argv[])
{
	if (signal(SIGTERM, sigShutdown) == SIG_ERR
	|| signal(SIGINT, sigShutdown) == SIG_ERR
	|| signal(SIGHUP, sigShutdown) == SIG_ERR
	|| signal(SIGPIPE, SIG_IGN) == SIG_ERR)
	{
		std::cerr << "signal() setup failed" << std::endl;
		return 1;
	}
	
	if (argc != 3) {
		std::cout << "Usage: ./ircserv <port> <password>" << std::endl;
		return 2;
	}
	
	try {
		Config  config(argv[1], argv[2]);
		Reactor	pollLoop;
		Server	server(std::move(config));
		pollLoop.registerHandler(&server);
		g_servPtr = &server;
		
		pollLoop.run();
		
	} catch (const std::exception& e) {
		std::cerr << "Exception caught in main: " << e.what() << std::endl;
		return 3;
	}
	return 0;
}
