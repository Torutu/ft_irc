/*	Insert 42 Header	*/

#include "../inc/Socket.hpp"
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>

Socket::Socket()
  : fd_(::socket(AF_INET, SOCK_STREAM, 0))
{
	if (fd_ < 0)
		throw std::system_error(errno, std::generic_category(), "socket() failed");
	int flags = fcntl(fd_, F_GETFL, 0);
	if (flags < 0 || fcntl(fd_, F_SETFL, flags | O_NONBLOCK) < 0)
		throw std::system_error(errno, std::generic_category(), "failed to set non‑blocking");
}

Socket::Socket(int fd) noexcept
  : fd_(fd)
{}

Socket::~Socket() {
	if (fd_ >= 0) ::close(fd_);
}

Socket::Socket(Socket&& o) noexcept
  : fd_(o.fd_), addr_(o.addr_)
{ o.fd_ = -1; }

Socket& Socket::operator=(Socket&& o) noexcept {
	if (this != &o) {
		if (fd_ >= 0) ::close(fd_);
		fd_      = o.fd_;
		addr_    = o.addr_;
		o.fd_    = -1;
	}
	return *this;
}

void Socket::bind(uint16_t port) {
	std::memset(&addr_, 0, sizeof(addr_));
	addr_.sin_family   = AF_INET;
	addr_.sin_addr.s_addr = INADDR_ANY;
	addr_.sin_port     = htons(port);
	if (::bind(fd_, reinterpret_cast<sockaddr*>(&addr_), sizeof(addr_)) < 0)
		throw std::system_error(errno, std::generic_category(), "bind() failed");
}

void Socket::listen(int backlog) {
	if (::listen(fd_, backlog) < 0)
		throw std::system_error(errno, std::generic_category(), "listen() failed");
}

Socket Socket::accept() {
	sockaddr_in cli{};
	socklen_t sz = sizeof(cli);
	int clientFd = ::accept(fd_, reinterpret_cast<sockaddr*>(&cli), &sz);
	if (clientFd < 0)
		throw std::system_error(errno, std::generic_category(), "accept() failed");
	// newly accepted socket inherits non‑blocking from listening socket on Linux
	return Socket(clientFd);
}

ssize_t Socket::send(std::string_view data) const {
	throwIfInvalid("send");
	size_t sent = 0;
	while (sent < data.size()) {
		ssize_t n = ::send(fd_, data.data() + sent, data.size() - sent, 0);
		if (n < 0) {
			if (errno==EAGAIN||errno==EWOULDBLOCK) break;
			throw std::system_error(errno, std::generic_category(), "send() failed");
		}
		sent += n;
	}
	return sent;
}

ssize_t Socket::receive(std::string& buf) const {
	throwIfInvalid("recv");
	char tmp[4096];
	ssize_t n = ::recv(fd_, tmp, sizeof(tmp), 0);
	if (n < 0) {
		if (errno==EAGAIN||errno==EWOULDBLOCK) return 0;
		throw std::system_error(errno, std::generic_category(), "recv() failed");
	}
	if (n>0) buf.assign(tmp, n);
	return n;
}

void Socket::throwIfInvalid(const char* what) const {
	if (fd_ < 0)
		throw std::logic_error(std::string(what) + " on closed socket");
}
