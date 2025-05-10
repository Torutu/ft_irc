#include "../inc/irc.hpp"

void Server::handleClientRead(size_t index) {
	char buffer[IRC_BUFFER_SIZE];

	int fd = pollFds_.at(index).fd;
	
	ssize_t bytesRead = recv(fd, buffer, sizeof(buffer), 0);
	if (bytesRead <= 0) {
		handleClientError(bytesRead == 0 ? 0 : errno, index);
		return;
	}
	std::string&	input = clients_.at(fd).recvBuf_;//in case recv with a partial message, we stored it in recvBuf_ so we have to add the new msg to it
	if (input.size() + bytesRead > IRC_BUFFER_SIZE) {
		std::cerr << "recvBuff filling up! Data lost! Client fd:" << fd << "pollFd_[] index:" << index << std::endl;
		return;
	}
	input.append(buffer, bytesRead);
	// Split the input by \r\n and process each command
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
	authenticated{false},
	nickReceived{false},
	userReceived{false},
	passReceived{false},
	modeReceived{false},
	whois{false},
	sendBuf_{},
	recvBuf_{}
	// bytesRecvd_{0},
{
	sendBuf_.reserve(IRC_BUFFER_SIZE);
	recvBuf_.reserve(IRC_BUFFER_SIZE);
}

Client::Client(Socket&& so)  // Parameterized constructor
:	so_{std::move(so)},
	nick{"guest"},
	user{"guest"},
	authenticated{false},
	nickReceived{false},
	userReceived{false},
	passReceived{false},
	modeReceived{false},
	whois{false},
	sendBuf_{},
	recvBuf_{}
	// bytesRecvd_{0},
{
	sendBuf_.reserve(IRC_BUFFER_SIZE);
	recvBuf_.reserve(IRC_BUFFER_SIZE);
}

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

bool	Client::bufForSending(const std::string& data) {
	if (data.empty()){
		return true;
	}
	if (sendBuf_.size() + data.size() > IRC_BUFFER_SIZE) {
		std::cerr << "sendBuf_ is filling up" << std::endl;
		return false;
	}
	try {
		sendBuf_.append(data);
	} catch (std::exception& e) {
		std::cerr << "sendBuf append failed: " << e.what() << std::endl;
		return false;
	}
	return true;
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

