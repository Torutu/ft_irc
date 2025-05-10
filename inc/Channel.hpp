#pragma once

#include <string>
#include <vector>
#include <algorithm>

#include "Client.hpp"

class Channel
{
    private:
        std::string name;
        std::string topic;
        std::string pwd;
        std::vector<int> invitedUsers;
        std::vector<int> clients; // List of client file descriptors in the channel
        std::vector<int> operators; // List of operator file descriptors in the channel
        
        int userLimit;
        bool inviteOnly; // i
        bool topicRestrictedToOperators;  // t
        std::map<int, Client>* clientsPtr_;//should this be smart pointer?

    public:
        ~Channel();
        Channel();
        Channel(const std::string &name, std::map<int, Client>* allClientsPtr);
        Channel(const Channel &other);

        Channel &operator=(const Channel &src);

        const std::string& getName() const;
        const std::string& getTopic() const;
        const std::string& getPwd() const;

        int  getUserLimit() const;
        int getClientFdByNick(const std::string& nickname, const std::map<int, Client>& clients) const;


        bool getInviteOnly() const;
        bool getIsUserInvited(const int& fd) const;
        bool getTopicRestricted() const;
        bool isOperator(int fd) const; // Check if a client is an operator in the channel
        

        void addOperator(int fd); // Add a client as an operator in the channel
        void removeOperator(int fd); // Remove a client as an operator in the channel
        void setPassword(const std::string& newPassword);    
        void setUserLimit(int limit);
        void setInviteOnly(bool inviteOnly);
        void setTopicRestrictedToOperators(bool restricted);
        void addInvitedUser(const int& client_fd);
        void removeInvitedUser(const int& client_fd);
        void setTopic(const std::string& newTopic);
        void addClient(int fd); // Add a client to the channel
        void removeClient(int fd); // Remove a client from the channel
        void broadcast(const std::string& message, const std::string& sender_nick, int except_fd = -1); // Send a message to all clients in the channel except the sender
};