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

Client::Client(Socket&& so) : so_{std::move(so)}, nick{"guest"}, user{"guest"} // Parameterized constructor
{
	this->authenticated = false;
	this->nickReceived = false;
	this->userReceived = false;
	this->passReceived = false;
	this->modeReceived = false;
	this->whois = false;
}
//move constructor
Client::Client(Client&& other) noexcept
	:	so_{std::move(other.so_)},
		nick{other.nick},
		user{other.user},
		joinedChannels{std::move(other.joinedChannels)},
		authenticated{std::exchange(other.authenticated, false)},
		nickReceived{std::exchange(other.nickReceived, false)},
		userReceived{std::exchange(other.userReceived, false)},
		passReceived{std::exchange(other.passReceived, false)},
		modeReceived{std::exchange(other.modeReceived, false)},
		whois{std::exchange(other.modeReceived, false)}
{
	other.nick.clear();
	other.user.clear();
}

//move assignment
Client&	Client::operator=(Client&& other) noexcept {
	if (this != &other) {
		nick = other.nick;
		other.nick.clear();
		user = other.user;
		other.user.clear();
		joinedChannels = std::move(other.joinedChannels);
		authenticated = std::exchange(other.authenticated, false);
		nickReceived = std::exchange(other.nickReceived, false);
		userReceived = std::exchange(other.userReceived, false);
		passReceived = std::exchange(other.passReceived, false);
		modeReceived = std::exchange(other.modeReceived, false);
		whois = std::exchange(other.modeReceived, false);
	}
	return *this;
}

// Client::Client(std::string nick, std::string user, int fd): fd(fd), nick(nick), user(user) {}

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

