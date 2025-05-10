#include "../../inc/irc.hpp"

void Server::cmdTopic(int fd, const std::string& message) {
    std::istringstream iss(message);
    std::string command, channel, topic;
    iss >> command >> channel;

    if (channel.empty()) {
        ft_send(fd, ERR_NEEDMOREPARAMS);
        return;
    }

    Client &client = clients_[fd];

    if (channels_.find(channel) == channels_.end()) {
        ft_send(fd, ERR_NO_SUCH_CHANNEL(channel));
        return;
    }

    Channel &ch = channels_[channel];

    if (!client.isInChannel(channel)) {
        ft_send(fd, ERR_NOT_IN_CHANNEL(channel));
        return;
    }

    std::getline(iss, topic); // Get the rest if setting a topic
    if (!topic.empty() && topic[0] == ' ') topic = topic.substr(1);
    if (!topic.empty() && topic[0] == ':') topic = topic.substr(1);

    if (topic.empty()) {
        // just viewing
        if (ch.getTopic().empty())
            ft_send(fd, RPL_NOTOPIC(client.getNick(), channel));
        else
            ft_send(fd, RPL_TOPIC(client.getNick(), channel, ch.getTopic()));
    } else {
        // setting topic
        if (!client.isOperator(channel)) {
            ft_send(fd, ERR_CHANOPRIVSNEEDED(channel));
            return;
        }

        ch.setTopic(topic);
        std::string broadcast = ":" + client.getNick() + " TOPIC " + channel + " :" + topic + "\r\n";
        ch.broadcast(broadcast, client.getNick(), fd);
    }
}
