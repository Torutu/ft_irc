#include "../inc/irc.hpp"

Channel::Channel()
: name(""), topic(""), pwd(""), userLimit(-1), inviteOnly(false), topicRestrictedToOperators(true) {}

Channel::Channel(const std::string &name, std::map<int, Client>* allClientsPtr)
: name(name), topic(""), pwd(""), userLimit(-1), inviteOnly(false), topicRestrictedToOperators(true), clientsPtr_(allClientsPtr) {}

Channel::~Channel() {}

Channel::Channel(const Channel &other)  
{
    *this = other;
}

Channel &Channel::operator=(const Channel &other)
{
    if (this != &other)
    {
        this->name = other.getName();
        this->topic = other.getTopic();
        this->pwd = other.getPwd();
        this->userLimit = other.getUserLimit();
        this->inviteOnly = other.getInviteOnly();
        this->topicRestrictedToOperators = other.getTopicRestricted();
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

void Channel::broadcast(int sender_fd, const std::string& message, const std::string& sender_nick, int except_fd) // send to all clients except the sender
{(void)except_fd;
    std::cout << "broadcast" << std::endl;
    (void) sender_fd; // Unused parameter, can be removed if not needed
    std::string fullMessage = ":" + sender_nick + " PRIVMSG " + this->name + " :" + message + "\r\n";
    // for (std::vector<int>::iterator it = this->clients.begin(); it != this->clients.end(); ++it) // IMHO we don't gain anything by using an iterator on an int vector
    for (unsigned long i = 0; i < clients.size(); i++)
    {
        if (!clientsPtr_) std::cout << "null" << std::endl;
        if (clientsPtr_) {  // Check for null
            for (auto it = clientsPtr_->begin(); it != clientsPtr_->end(); ++it) {
                std::cout << "Key: " << it->first << ", Value: " << it->second.getFd() << std::endl;
            }
        }
        // auto it = clientsPtr_->find(clients.at(i));
        // std::cout << "auto it = clientsPtr_->find(clients.at(i));" << std::endl;
        // if (clients.at(i) != except_fd)
        // {
        //     // send(*it, fullMessage.c_str(), fullMessage.length(), 0); // we only directly call send() in the handleClientWrite() when the socket is ready for POLLOUT 
        //     try {
        //         if (it != clientsPtr_->end()) {
        //             std::cout << "auto it = clientsPtr_->find(clients.at(i));" << std::endl;
        //             clientsPtr_->at(clients.at(i)).bufForSending(fullMessage.c_str());
        //         }
        //     } catch (std::exception& e) {
        //         std::cerr << "channel broadcast - accessing Client map at key: " << i << " failed." << e.what() << std::endl;
        //     }
        //     std::cout << " end" << std::endl;
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
