#include "../../inc/irc.hpp"


void Server::cmdJoin(int fd, const std::string& message) {
    std::istringstream iss(message);
    std::string command, channel;
    iss >> command >> channel;

    if (channel.empty()) {
        std::string errMsg = ERR_NEEDMOREPARAMS;
        ft_send(fd, errMsg);
        return;
    }

    if (channels_.find(channel) == channels_.end()) {
        channels_[channel] = Channel(channel, &clients_); // Create a new channel if it doesn't exist
    }

    if (clients_[fd].isInChannel(channel)) {
        std::string errMsg = ERR_USERONCHANNEL(channel);
        ft_send(fd, errMsg);
        return;
    }

    clients_[fd].joinChannel(channel, false); // Your client-side method
    channels_[channel].addClient(fd);         // Add to channel list

    // Notify all users in the channel (except the one joining)
    std::string joinMessage = ":" + clients_[fd].getNick() + " JOIN :" + channel + "\r\n";
    channels_[channel].broadcast(joinMessage, clients_[fd].getNick(), -1); // Send to all
}

