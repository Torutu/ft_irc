#pragma once


class EventHandler {
public:
	virtual	bool	send() = 0;
	virtual	bool	receive() = 0;
	// virtual	void	error(int fd) = 0;
};