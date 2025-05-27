
#include "../inc/Socket.hpp"
#include "../inc/Server.hpp"
using std::string;
using std::cerr;
using std::cout;
using std::endl;

Socket::Socket()
:	fd_{-1},
	addr_{},
	isListening_{false}
{}

Socket::Socket(int fd, sockaddr_in addr)
:	fd_{fd}, addr_{addr}, isListening_{false} {
	if (fd_ < 0) {
		throw std::system_error(errno, std::generic_category(), "socket(fd, addr) must have positive fd");
	}
}

Socket::Socket(Socket&& other) noexcept
: fd_{other.fd_}, addr_{other.addr_}, isListening_{other.isListening_}
{
	other.fd_ = -1;
	other.addr_ = {};
	other.isListening_ = false;
}

Socket&	Socket::operator=(Socket&& other) noexcept {
	if (this != &other) {
		if (fd_ >= 0) {
			if (::close(fd_) < 0) {
				cerr << "Failed to close fd " << fd_ << ": " << std::strerror(errno) << endl;
			}
		}
		fd_ = std::exchange(other.fd_, -1);
		addr_ = std::move(other.addr_);
		isListening_ = std::exchange(other.isListening_, false);
	
		other.addr_ = {};
	}
	return *this;
}

Socket::~Socket() noexcept {
	if (fd_ >= 0) {
		if (::close(fd_) < 0) {
			cerr << "Failed to close fd " << fd_ << ": " << std::strerror(errno) << endl;
		}
		fd_ = -1;
	}
}

void	Socket::initListener(uint16_t port) {
	if (port < 1024) {
		throw std::logic_error("Listener port must be valid and not between 1-1023");
	}

	fd_ = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
	if (fd_ < 0) {
		throw std::system_error(errno, std::generic_category(), "socket() failed");
	}

	int opt = 1;
	if (::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		throw std::system_error(errno, std::generic_category(), "setsockopt(SO_REUSEADDR) failed");
	}

	addr_ = {};
	addr_.sin_family      = AF_INET;
	addr_.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_.sin_port        = htons(port);

	if (::bind(fd_, reinterpret_cast<sockaddr*>(&addr_), sizeof(addr_)) < 0) {
		throw std::system_error(errno, std::generic_category(), "bind() failed");
	}
	if (::listen(fd_, SOMAXCONN) < 0) {
		throw std::system_error(errno, std::generic_category(), "listen() failed");
	}
	isListening_ = true;
}


bool	Socket::setNonBlocking(int fd) {
	int flags = 0;

	while ((flags = fcntl(fd, F_GETFL, 0)) < 0) {
		cerr << "fcntl() error: " << strerror(errno) << endl;
		if (errno == EINTR) {
			continue;
		}
		return false;
	}
	while (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
		cerr << "fcntl() error: " << strerror(errno) << endl;
		if (errno == EINTR) {
			continue;
		}
		return false;
	}
	return true;
}

bool	Socket::accept(Socket& toSocket) const {
	if (!isListening_) {
		return false;
	}

	sockaddr_in clientAddr{};
	socklen_t addrLen = sizeof(clientAddr);
	int clientFd = ::accept(fd_, reinterpret_cast<sockaddr*>(&clientAddr), &addrLen);
	if (clientFd < 0) {
		return false;
	}

	if (setNonBlocking(clientFd) == false) {
		return false;
	}

	toSocket = Socket(clientFd, clientAddr);
	return true;
}

std::string	Socket::getIpStr() const {
	char ipStr[INET_ADDRSTRLEN] = {};

	if (inet_ntop(AF_INET, &addr_.sin_addr, ipStr, INET_ADDRSTRLEN) == nullptr) {
		cerr << "inet_ntop error: " << strerror(errno) << endl;
	}
	return ipStr;
}
