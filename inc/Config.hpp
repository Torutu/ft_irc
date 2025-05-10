
#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <string>
#include <system_error>
#include <utility>//std::exchange
#include <cctype>//std::isalnum

class Config {
	int					port_;
	std::string			portStr_;
	std::string			password_;
	const std::string	serverName_ = "ft_irc, WB edition ®";
	const unsigned short	minPassLen_ = 4;
	const std::string	allowedPassChars_ = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_!?#";

	void	validate_();
	bool	isValidPort_();
	bool	isValidPassword_();

public:
	Config();
	Config(std::string listenPort, std::string passw);
	Config(const Config&)				= delete;
	Config&	operator=(const Config&)	= delete;
	Config(Config&&) noexcept;
	Config&	operator=(Config&& other) noexcept;
	~Config()							= default;

	int					getPort() const noexcept {return port_;}
	const std::string&	getServName() const noexcept {return serverName_;}
	bool				CheckPassword(const std::string& input) const noexcept {return input == password_;}

	const std::string&	getAllowPassChars()	const noexcept {return allowedPassChars_;}
	unsigned short		getMinPassLen() const noexcept {return minPassLen_;}
};

#endif
