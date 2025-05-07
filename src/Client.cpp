#include "../inc/irc.hpp"

void Server::handleClient(size_t index) {
	char buffer[1024];
	ssize_t bytesRead = recv(pollFds_[index].fd, buffer, sizeof(buffer) - 1, 0);
	if (bytesRead <= 0) {
		handleClientError(bytesRead == 0 ? 0 : errno, index);
		return;
	}
	
	buffer[bytesRead] = '\0'; // Null-terminate
	int fd = pollFds_[index].fd;
	// Split the input by \r\n and process each command
	std::string input(buffer);
	size_t pos = 0;
	std::string line;
	while ((pos = input.find("\r\n")) != std::string::npos) {
		line = input.substr(0, pos);
		processCommand(fd, line);
		input.erase(0, pos + 2); // Move past \r\n
	}
}

Client::Client()
:	so_{},
	nick{"guest"},
	user{"guest"},
	bytesSent_{0},
	bytesRecvd_{0},
	authenticated{false},
	nickReceived{false},
	userReceived{false},
	passReceived{false},
	modeReceived{false},
	whois{false},
	sendBuf_{},
	recvBuf_{}
{}

Client::Client(Socket&& so)  // Parameterized constructor
:	so_{std::move(so)},
	nick{"guest"},
	user{"guest"},
	bytesSent_{0},
	bytesRecvd_{0},
	authenticated{false},
	nickReceived{false},
	userReceived{false},
	passReceived{false},
	modeReceived{false},
	whois{false},
	sendBuf_{},
	recvBuf_{}
{}

//move constructor - UPDATE
Client::Client(Client&& other) noexcept
	:	so_{std::move(other.so_)},
		nick{std::move(other.nick)},
		user{std::move(other.user)},
		joinedChannels{std::move(other.joinedChannels)},
		authenticated{std::exchange(other.authenticated, false)},
		nickReceived{std::exchange(other.nickReceived, false)},
		userReceived{std::exchange(other.userReceived, false)},
		passReceived{std::exchange(other.passReceived, false)},
		modeReceived{std::exchange(other.modeReceived, false)},
		whois{std::exchange(other.modeReceived, false)},
		sendBuf_{std::move(other.sendBuf_)},
		recvBuf_{std::move(other.recvBuf_)}
{}

//move assignment - UPDATE
Client&	Client::operator=(Client&& other) noexcept {
	if (this != &other) {
		nick = std::exchange(other.nick, "");
		user = std::exchange(other.user, "");
		joinedChannels = std::move(other.joinedChannels);
		authenticated = std::exchange(other.authenticated, false);
		nickReceived = std::exchange(other.nickReceived, false);
		userReceived = std::exchange(other.userReceived, false);
		passReceived = std::exchange(other.passReceived, false);
		modeReceived = std::exchange(other.modeReceived, false);
		whois = std::exchange(other.modeReceived, false);
		sendBuf_ = std::exchange(other.sendBuf_, "");
		recvBuf_ = std::exchange(other.recvBuf_, "");
	}
	return *this;
}

// Client::Client(std::string nick, std::string user, int fd): fd(fd), nick(nick), user(user) {}

ssize_t Client::send(std::string_view data) const {
	sendBuf_ = data.data();
	ssize_t totalSent = 0;
	while (totalSent < data.size()) {
		ssize_t n = ::send(fd_, data.data() + totalSent, data.size() - totalSent, MSG_NOSIGNAL);
		if (n > 0) {
			totalSent += n;
		} else if (n == -1) {
			if (errno == EINTR) {
				continue;  // Interrupted; retry.
			}
			else if (errno == EAGAIN || errno == EWOULDBLOCK) {
				break;  // Non-fatal, temporary conditions.
			}
			else {
				throw std::system_error(errno, std::generic_category(), "write() failed");// Fatal error
			}
		} else {
			break; // n == 0; break out to avoid potential infinite loop.
		}
	}
	return totalSent;
}

ssize_t Client::receive(std::string &buf) const {
	char tmp[4096];
	ssize_t n = 0;

	while (true) {
		n = ::read(fd_, tmp, sizeof(tmp));
		if (n > 0) {
			break;
		} else if (n == 0) {
			break;// End-of-file: the peer has closed the connection.
		} else {
			if (errno == EINTR) { // Interrupted by a signal, retry the read.
				continue;
			} else if (errno == EAGAIN || errno == EWOULDBLOCK) { // no data available; return 0 bytes read.
				n = 0;
				break;
			} else {
				throw std::system_error(errno, std::generic_category(), "read() failed");
			}
		}
	}

	if (n > 0) {
		buf.assign(tmp, static_cast<size_t>(n));
	} else {
		buf.clear();
	}
	return n;
}

bool Client::isInChannel(const std::string &channel)
{
	if (joinedChannels.find(channel) != joinedChannels.end())
		return true;
	return false;
}

bool Client::getOperator(const std::string &channel)
{
	if(isInChannel(channel))
		return joinedChannels[channel];
	return false;
}

void Client::setNick(const std::string &nick)
{
	this->nick = nick;
}

void Client::setUser(const std::string &user)
{
	this->user = user;
}
void Client::setNickReceived()
{
	this->nickReceived = true;
}
void Client::setUserReceived()
{
	this->userReceived = true;
}
void Client::setPassReceived()
{
	this->passReceived = true;
}
void Client::setModeReceived()
{
	this->modeReceived = true;
}
void Client::setAuthenticated()
{
	this->authenticated = true;
}
void Client::joinChannel(const std::string &channel, bool is_operator)
{
	joinedChannels[channel] = is_operator;
}

void Client::leaveChannel(const std::string &channel)
{
	joinedChannels.erase(channel);
}
bool Client::isOperator(const std::string &channel)
{
	if (joinedChannels.find(channel) != joinedChannels.end())
		return true;
	return false;
}
void Client::setOperator(const std::string &channel, bool is_operator)
{
	if (isInChannel(channel))
		joinedChannels[channel] = is_operator;
}

std::string Client::getNick() const
{
	return this->nick;
}

std::string Client::getUser() const
{
	return this->user;
}

bool Client::hasReceivedNick() const
{
	return this->nickReceived;
}

bool Client::hasReceivedUser() const
{
	return this->userReceived;
}

bool Client::hasReceivedPass() const
{
	return this->passReceived;
}

bool Client::hasReceivedMode() const
{
	return this->modeReceived;
}

bool Client::isAuthenticated() const
{
	return this->authenticated;
}

int Client::getFd() const
{
	return this->so_.getFd();
}

const std::map<std::string, bool>& Client::getJoinedChannels() const
{
	return this->joinedChannels;
}

