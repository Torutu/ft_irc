#pragma once

class EventHandler {
	virtual	void	handleEvent(int fd) = 0;
};