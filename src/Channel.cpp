#include "../inc/irc.hpp"

Channel::Channel()
: name(""), topic(""), pwd(""), userLimit(-1), inviteOnly(false), topicRestrictedToOperators(true) {}

Channel::Channel(const std::string &name, std::map<int, Client>* allClientsPtr)
: name(name), topic(""), pwd(""), userLimit(-1), inviteOnly(false), topicRestrictedToOperators(true), clientsPtr_(allClientsPtr) {}

Channel::~Channel() {}

Channel::Channel(const Channel &other)
    : name(other.name),
      topic(other.topic),
      pwd(other.pwd),
      invitedUsers(other.invitedUsers),
      clients(other.clients),
      operators(other.operators),
      userLimit(other.userLimit),
      inviteOnly(other.inviteOnly),
      topicRestrictedToOperators(other.topicRestrictedToOperators),
      clientsPtr_(other.clientsPtr_)
{}

Channel &Channel::operator=(const Channel &other)
{
    if (this != &other)
    {
        this->name = other.name;
        this->topic = other.topic;
        this->pwd = other.pwd;
        this->invitedUsers = other.invitedUsers;
        this->clients = other.clients;
        this->operators = other.operators;
        this->userLimit = other.userLimit;
        this->inviteOnly = other.inviteOnly;
        this->topicRestrictedToOperators = other.topicRestrictedToOperators;
        this->clientsPtr_ = other.clientsPtr_;  // Copy the pointer as well!
    }
    return *this;
}

////////////////////////////////////////////////////////////////////////////

const std::string& Channel::getName() const
{
    return this->name;
}

const std::string& Channel::getTopic() const
{
    return this->topic;
}

const std::string& Channel::getPwd() const
{
    return this->pwd;
}

int Channel::getUserLimit() const
{
    return this->userLimit;
}

bool Channel::getInviteOnly() const
{
    return this->inviteOnly;
}

bool Channel::getIsUserInvited(const int& fd) const
{
    return std::find(this->invitedUsers.begin(), this->invitedUsers.end(), fd) != this->invitedUsers.end();
}

bool Channel::getTopicRestricted() const
{
    return this->topicRestrictedToOperators;
}

// In Channel.cpp
int Channel::getClientFdByNick(const std::string& nickname, const std::map<int, Client>& clients) const {
std::cout << "getCLientFDbyNick" << std::endl;
    for (std::map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); ++it){
        int fd = it->first;
        std::map<int, Client>::const_iterator cit = clients.find(fd);
        if (cit != clients.end() && cit->second.getNick() == nickname) {
            return fd;
        }
    }
    return -1; // Not found
}

/////////////////////////////////////////////////////////////////////////

void Channel::setPassword(const std::string& newPassword)
{
    this->pwd = newPassword;
}

void Channel::setUserLimit(int limit)
{
    this->userLimit = limit;
}

void Channel::setInviteOnly(bool inviteOnly)
{
    this->inviteOnly = inviteOnly;
}

void Channel::setTopicRestrictedToOperators(bool restricted)
{
    this->topicRestrictedToOperators = restricted;
}

////////////////////////////////////////////////////////////////

void Channel::addInvitedUser(const int& client_fd)
{
    if (std::find(this->invitedUsers.begin(), this->invitedUsers.end(), client_fd) == this->invitedUsers.end())
        this->invitedUsers.push_back(client_fd);
}

void Channel::removeInvitedUser(const int& client_fd)
{
    std::vector<int>::iterator it = std::find(this->invitedUsers.begin(), this->invitedUsers.end(), client_fd);
    if (it != this->invitedUsers.end())
        this->invitedUsers.erase(it);
}

void Channel::setTopic(const std::string& newTopic)
{
    this->topic = newTopic;
}

void Channel::addClient(int fd)
{
    if (std::find(this->clients.begin(), this->clients.end(), fd) == this->clients.end())
        this->clients.push_back(fd);
}

void Channel::removeClient(int fd)
{
    std::vector<int>::iterator it = std::find(this->clients.begin(), this->clients.end(), fd);
    if (it != this->clients.end())
        this->clients.erase(it);
}

void Channel::broadcast(const std::string& message, const std::string& sender_nick, int except_fd) // send to all clients except the sender
{
    if (!clientsPtr_) {
        return;
    }
    std::string fullMessage = ":" + sender_nick + " PRIVMSG " + this->name + " :" + message + "\r\n";
    int cliFd = 0;
    for (unsigned long i = 0; i < clients.size(); i++)
    {
        cliFd = clients.at(i);
        auto it = clientsPtr_->find(cliFd);
        if (cliFd != except_fd && it != clientsPtr_->end())
        {
            try {
                clientsPtr_->at(cliFd).bufForSending(fullMessage.c_str());
            } catch (std::exception& e) {
                std::cerr << "channel broadcast - accessing Client map at key: " << i << " failed." << e.what() << std::endl;
            }
        }
    }
}


bool Channel::isOperator(int fd) const {
    return std::find(this->operators.begin(), this->operators.end(), fd) != this->operators.end();
}

void Channel::addOperator(int fd) {
    if (!isOperator(fd))
        this->operators.push_back(fd);
}

void Channel::removeOperator(int fd) {
    std::vector<int>::iterator it = std::find(this->operators.begin(), this->operators.end(), fd);
    if (it != this->operators.end())
        this->operators.erase(it);
}
