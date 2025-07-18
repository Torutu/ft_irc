#include "../inc/Server.hpp"
#include "../inc/Client.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::string;

Client::Client()
:	so_{},
	pfd_{nullptr},
	nick_{"unknown"},
	delimiter_{"\n"},
	authenticated{false},
	nickReceived{false},
	userReceived{false},
	passReceived{false},
	modeReceived{false},
	whois{false},
	authAttempts_{0},
	lastActive_{std::chrono::steady_clock::now()}
{
	sendBuf_.reserve(IRC_MAX_BUF);
	recvBuf_.reserve(IRC_MAX_BUF);
}

Client::Client(Socket&& so, pollfd *pfd)
:	so_{std::move(so)},
	pfd_{pfd},
	nick_{"unknown"},
	delimiter_{"\n"},
	authenticated{false},
	nickReceived{false},
	userReceived{false},
	passReceived{false},
	modeReceived{false},
	whois{false},
	authAttempts_{0},
	lastActive_{std::chrono::steady_clock::now()}
{
	sendBuf_.reserve(IRC_MAX_BUF);
	recvBuf_.reserve(IRC_MAX_BUF);
}

Client::Client(Client&& other) noexcept
	:	so_{std::move(other.so_)},
		pfd_{std::exchange(other.pfd_, nullptr)},
		sendBuf_{std::move(other.sendBuf_)},
		recvBuf_{std::move(other.recvBuf_)},
		nick_{std::move(other.nick_)},
		usrnm_{std::move(other.usrnm_)},
		delimiter_{std::move(other.delimiter_)},
		joinedChannels{std::move(other.joinedChannels)},
		authenticated{std::exchange(other.authenticated, false)},
		nickReceived{std::exchange(other.nickReceived, false)},
		userReceived{std::exchange(other.userReceived, false)},
		passReceived{std::exchange(other.passReceived, false)},
		modeReceived{std::exchange(other.modeReceived, false)},
		whois{std::exchange(other.modeReceived, false)},
		authAttempts_{std::exchange(other.authAttempts_, 0)},
		lastActive_{other.lastActive_}
{}

Client&	Client::operator=(Client&& other) noexcept {
	if (this != &other) {
		so_ = std::move(other.so_);
		pfd_ = std::move(other.pfd_);
		sendBuf_ = std::move(other.sendBuf_);
		recvBuf_ = std::move(other.recvBuf_);
		nick_ = std::move(other.nick_);
		usrnm_ = std::move(other.usrnm_);
		delimiter_ = std::move(other.delimiter_);
		joinedChannels = std::move(other.joinedChannels);
		authenticated = std::exchange(other.authenticated, false);
		nickReceived = std::exchange(other.nickReceived, false);
		userReceived = std::exchange(other.userReceived, false);
		passReceived = std::exchange(other.passReceived, false);
		modeReceived = std::exchange(other.modeReceived, false);
		whois = std::exchange(other.modeReceived, false);
		authAttempts_ = std::exchange(other.authAttempts_, 0);
		lastActive_ = other.lastActive_;
	}
	return *this;
}

void	Client::toSend(const std::string& data) {
	if (data.empty()) {
		return;
	}

	if (sendBuf_.size() + data.size() > IRC_MAX_BUF) {
		cerr << "sendBuf_ at fd " << so_.getFd() << " almost reached IRC_MAX_BUF " << IRC_MAX_BUF << " bytes and last message has been ignored." << endl;
		return;
	}

	sendBuf_.append(data);
	assert(pfd_);
	pfd_->events |= POLLOUT;
}

//if returns false client connection should be deleted
bool	Client::send() {
	if (sendBuf_.empty() == true) {
		return true;
	}

	ssize_t sent = ::send(so_.getFd(), sendBuf_.c_str(), sendBuf_.size(), MSG_NOSIGNAL);
	if (sent < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return true;
		} else {
			std::cerr << "Error with client:" << so_.getIpStr() << " FD: " << so_.getFd() << " send() error: " << strerror(errno) << std::endl;
			return false;
		}
	} else if (sent == 0) {
		std::cout << "Client disconnected: " << so_.getIpStr() << " (FD: " << so_.getFd() << ")" << std::endl;
		return false;
	}
	sendBuf_.erase(0, sent);

	if (sendBuf_.empty()) {
	assert(pfd_);
		pfd_->events &= ~POLLOUT;
	}

	lastActive_ = std::chrono::steady_clock::now();

	return true;
}

/*
	on returning false, the client connection in question should be closed.
	in case of partial data, Client stores it in recvBuf_ and we add new data to it to get a full message
*/
bool	Client::receive() {
	char buffer[IRC_BUFFER_SIZE + 1];

	ssize_t bytesRead = ::recv(so_.getFd(), buffer, sizeof(buffer) - 1, MSG_NOSIGNAL);
	if (bytesRead < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return true;
		} else {
			std::cerr << "Error with client:" << so_.getIpStr() << " FD: " << so_.getFd() << " recv() error: " << strerror(errno) << std::endl;
			return false;
		}
	} else if (bytesRead == 0) {
		std::cout << "receive() Client disconnected: " << so_.getIpStr() << " (FD: " << so_.getFd() << ")" << std::endl;
		return false;
	}

	buffer[bytesRead] = '\0';

	if (recvBuf_.size() + bytesRead > IRC_MAX_BUF) {
		std::cerr << "recvBuff filling up! Data lost! Client fd:" << so_.getFd() << std::endl;
		return false;
	}

	recvBuf_.append(buffer, bytesRead);
	if (authenticated == false && authAttempts_ <= (int)std::string("CAP LS\r\n").length()) {
		resolveDelimiter();
	}

	#ifdef IRC_BUF_PRINTS
		cout << YELLOWIRC << "recvBuf:" << recvBuf_ << RESETIRC << endl;
	#endif

	lastActive_ = std::chrono::steady_clock::now();

	return true;
}

std::string	Client::getMsgs() {
	size_t	pos = recvBuf_.rfind(delimiter_);
	std::string	completeMsgs;
	if (pos != std::string::npos) {
		completeMsgs = recvBuf_.substr(0, pos + delimiter_.length());
		recvBuf_.erase(0, pos + delimiter_.length());
	}
	return completeMsgs;
}

void	Client::resolveDelimiter() {
	size_t	posCr = recvBuf_.find('\r');
	size_t	posNl = recvBuf_.find('\n');

	if (posCr == std::string::npos && posNl == std::string::npos) {
		return;
	} else if (posCr != std::string::npos && posNl != std::string::npos
				&& posNl > posCr && posNl - posCr == 1) {
		delimiter_ = "\r\n";
	} else {
		delimiter_ = "\n";
	}
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

void Client::joinChannel(const std::string &channel, bool is_operator)
{
	if (is_operator)
	{
		joinedChannels[channel] = true;
	}
	else
	{
		joinedChannels[channel] = false;
	}
	channelCount_++;
}

void Client::leaveChannel(const std::string &channel)
{
	joinedChannels.erase(channel);
	channelCount_--;
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
