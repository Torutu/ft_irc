#pragma once

#include <unordered_map>
#include <iostream>
#include <cstring>
#include <vector>
#include <poll.h>
#include "EventHandler.hpp"

class Reactor {
	std::unordered_map<int, EventHandler*> handlers_;// Client:: accept, receive, Server::acceptNewConnection
	std::vector<struct pollfd> pollfds_;

public:
	void registerHandler(int fd, EventHandler* handler) {
		handlers_[fd] = handler;
	}

	void run() {
		while (state & RUNNING) {
			int nReady = poll(pollfds_.data(), pollfds_.size(), -1);
			if (nReady < 0) {
				for (int i = 0; i < pollfds_.size(); ++i) {
					if (pollfds_[i].revents & POLLIN) {
						handlers_[pollfds_[i].fd]->handleEvent(pollfds_[i].fd);
					}
				}
			} else {
				handlePollError();
				std::cerr << "poll() -1 with errno: " << strerror(errno) << "; pollfd vector size: " << pollfds_.size() << std::endl;
				continue;
			}
		}
	}
};