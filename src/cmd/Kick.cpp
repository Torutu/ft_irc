#include "../../inc/irc.hpp"

void Server::kickUser(int sender_fd, const std::string& channelName, const std::string& reason, const std::string& targetNick) {
    Channel& channel = channels_[channelName];
    int target_fd = channel.getClientFdByNick(targetNick, clients_);

    if (target_fd == -1)
        return; // Shouldn't happen if validated earlier

    std::string message = ":" + clients_[sender_fd].getNick() + " KICK " + channelName + " " + targetNick + " :" + reason + "\r\n";

    channel.broadcast(message, clients_[sender_fd].getNick());

    // Remove user from channel
    channel.removeClient(target_fd);
    clients_[target_fd].leaveChannel(channelName);
}

// KICK command implementation
void Server::cmdKick(int sender_fd, const std::vector<std::string>& params) {
    if (params.size() < 2) {
        ft_send(sender_fd, ERR_NEEDMOREPARAMS);
        return;
    }

    const std::string& channelName = params[0];
    const std::string& targetNick = params[1];

    if (channels_.find(channelName) == channels_.end()) {
        ft_send(sender_fd, ERR_NO_SUCH_CHANNEL(channelName));
        return;
    }

    Channel& channel = channels_[channelName];

    if (!channel.isOperator(sender_fd)) {
        ft_send(sender_fd, ERR_CHANOPRIVSNEEDED(channelName));
        return;
    }

    int target_fd = channel.getClientFdByNick(targetNick, clients_);
    if (target_fd == -1) {
        ft_send(sender_fd, ERR_USERNOTINCHANNEL(targetNick, channelName));
        return;
    }

    std::string reason = targetNick;
    if (params.size() > 2) {
        reason.clear();
        for (size_t i = 2; i < params.size(); ++i) {
            if (i > 2)
                reason += " ";
            reason += params[i];
        }
    }

    kickUser(sender_fd, channelName, reason, targetNick);
}
