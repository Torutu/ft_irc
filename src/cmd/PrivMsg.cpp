#include "../../inc/irc.hpp"

void Server::cmdPrivMsg(int fd, const std::string& message) {
    std::istringstream iss(message);
    std::string command, target, msgPart;
    iss >> command >> target;

    if (target.empty()) {
        ft_send(fd, ERR_NO_RECIPIENT);
        return;
    }

    std::getline(iss, msgPart); // everything after the target

    if (msgPart.empty() || msgPart == " :" || msgPart == ":") {
        ft_send(fd, ERR_NO_RECIPIENT);
        return;
    }

    // Clean up the message part
    if (msgPart[0] == ' ')
	{
        msgPart = msgPart.substr(1);
	}
    if (msgPart[0] == ':')
	{
        msgPart = msgPart.substr(1);
	}

	std::string fullMsg = msgPart + "\r\n";

    if (target[0] == '#') {
        // It's a channel message
        if (channels_.find(target) == channels_.end()) {
            ft_send(fd, ERR_NO_SUCH_CHANNEL(target));
            return;
        }
        if (!clients_[fd].isInChannel(target)) {
            ft_send(fd, ERR_NOT_IN_CHANNEL(target));
            return;
        }

        channels_[target].broadcast(fullMsg, clients_[fd].getNick(), fd);
    } else {
        // It's a private message to a user
        int targetFd = getClientFdByNick(target);
        if (targetFd == -1) {
            ft_send(fd, ERR_NO_SUCH_CHANNEL(target)); // You may want to define ERR_NOSUCHNICK
            return;
        }
        ft_send(targetFd, fullMsg);
    }
}
