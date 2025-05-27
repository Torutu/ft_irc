#pragma once

#include <unordered_map>
#include <iostream>
#include <cstring>
#include <vector>
#include <poll.h>
#include "EventHandler.hpp"

class Reactor {
	std::unordered_map<int, EventHandler*> handlers_;// Client:: send, receive, Server::acceptNewConnection
	std::vector<struct pollfd> pollfds_;

public:
	void registerHandler(int fd, EventHandler* handler) {
		try {
			handlers_[fd] = handler;
		} catch (std::exception& e) {
			std::cerr << "registerHandler() exception: " << e.what() << std::endl;
		}
	}

	void run() {
		while (state & RUNNING) {
			int nReady = poll(pollfds_.data(), pollfds_.size(), -1);
			if (nReady < 0) {
				for (int i = pollfds_.size() - 1; i >= 0 ; i--) {
					if (pollfds_[i].revents & POLLIN) {
						handlers_[pollfds_[i].fd]->receive();
					}
					if (pollfds_[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
						handlers_[pollfds_[i].fd]->error(pollfds_[i].fd);
					} else if (pollfds_[i].revents & POLLOUT) {
						handlers_[pollfds_[i].fd]->send();
					}
				}
			} else {
				std::cerr << "poll() -1 with errno: " << strerror(errno) << "; pollfd vector size: " << pollfds_.size() << std::endl;
				continue;
			}
		}
	}
};