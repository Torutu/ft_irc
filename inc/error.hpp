#pragma once

class Server; // forward declare

// Common error
#define ERR_NOT_IN_CHANNEL(chan) (":localhost 442 " + chan + " :You're not on that channel\r\n")
#define ERR_NO_SUCH_CHANNEL(chan) (":localhost 403 " + chan + " :No such channel\r\n")
#define ERR_CHANOPRIVSNEEDED(chan) (":localhost 482 " + chan + " :You're not channel operator\r\n")
#define ERR_NOT_IN_CHANNEL(chan) (":localhost 442 " + chan + " :You're not on that channel\r\n")

// NICK
#define ERR_NO_NICKNAME(nick) (":localhost 431 " + nick + " :No nickname given\r\n")
#define ERR_NICK_IN_USE(nick) (":localhost 433 " + nick + " :Nickname is already in use\r\n")
#define RPL_WELCOME(nick)     (":localhost 001 " + nick + " :Welcome to the server\r\n")

// JOIN
#define ERR_NEEDMOREPARAMS ":localhost 461 JOIN :Not enough parameters\r\n"
#define ERR_USERONCHANNEL(chan) (":localhost 443 " + chan + " :You are already on that channel\r\n")

// PRIVMSG
#define ERR_NO_RECIPIENT ":localhost 411 :No recipient given\r\n"



// USER
#define ERR_NOT_ENOUGH_PARAMS ":localhost 461 USER :Not enough parameters\r\n"
#define ERR_ALREADY_REGISTERED ":localhost 462 USER :You may not reregister\r\n"
#define WELCOME_MSG(user) (":localhost 001 " + user + " :Welcome to the server\r\n")

// PING
#define ERR_NO_PING_TARGET ":localhost 409 :No target for PING\r\n"

// TOPIC

#define RPL_NOTOPIC(nick, chan) (":localhost 331 " + nick + " " + chan + " :No topic is set\r\n")
#define RPL_TOPIC(nick, chan, topic) (":localhost 332 " + nick + " " + chan + " :" + topic + "\r\n")

// MODE
#define ERR_UNKNOWNMODE(mode) (":localhost 472 " + mode + " :is unknown mode char to me\r\n")
#define ERR_NO_PRIVILEGES ":localhost 481 :Permission Denied- You're not an IRC operator\r\n"
#define ERR_NOTREGISTERED ":localhost 451 :You have not registered\r\n"
#define ERR_NOSUCHNICK(nick) (":localhost 401 " + nick + " :No such nick/channel\r\n")

// KICK
#define ERR_USERNOTINCHANNEL(nick, chan) (":localhost 441 " + nick + " " + chan + " :They aren't on that channel\r\n")
