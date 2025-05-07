#ifndef SOCKET_HPP
# define SOCKET_HPP

#include <unistd.h>
#include <cstring>
#include <string>
#include <string_view>
#include <iostream>
#include <netinet/in.h>
#include <system_error>
#include <sys/socket.h> //SOMAXCONN, listen()
#include <cassert>
#include <utility>//std::exchange

/**
 * @brief RAII wrapper for a non‑blocking TCP/IPv4 socket.
 *
 * On construction, creates a socket and sets it non‑blocking.
 * Closes on destruction.
 * Throws on failures.
 * Throws on failures.
 */
 class Socket {
	int			fd_;
	sockaddr_in	addr_;
	bool		isListening_;

public:
	Socket();
	explicit Socket(int fd, sockaddr_in addr);
	Socket(const Socket& other)				= delete;
	Socket& operator=(const Socket& other)	= delete;
	Socket(Socket&& other) noexcept;
	Socket& operator=(Socket&& other) noexcept;
	~Socket() noexcept;

	bool	accept(Socket& toSocket) const;
	void	makeListener(uint16_t port);

	int			getFd() const noexcept {return fd_;}
	sockaddr_in getAddr() const noexcept {return addr_;}
	bool		isListener() const noexcept {return isListening_;}
};

#endif
