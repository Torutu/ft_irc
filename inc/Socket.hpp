/*	Insert 42 header	*/

/* Socket class */
#include <string>
#include <string_view>
#include <netinet/in.h>
#include <system_error>
#include <sys/socket.h> //SOMAXCONN, listen()
#include <cassert>

/**
 * @brief RAII wrapper for a non‑blocking TCP/IPv4 socket.
 *
 * On construction, creates a socket and sets it non‑blocking.
 * Closes on destruction.
 * Throws on failure.
 */
class Socket {
public:
	/** Create a non‑blocking TCP socket. @throws on failure. */
	Socket();

	/** Wrap an existing non‑blocking socket FD. */
	explicit Socket(int fd) noexcept;

	~Socket();

	Socket(const Socket&) = delete;
	Socket& operator=(const Socket&) = delete;

	Socket(Socket&&) noexcept;
	Socket& operator=(Socket&&) noexcept;

	/** Bind to all interfaces at `port`. @throws on failure. */
	void bind(uint16_t port);

	/** Start listening with given backlog. @throws on failure. */
	void listen(void);

	/** Accept a new client connection as a non‑blocking socket. */
	Socket accept();///???

	/**
	 * @brief Send all of `data`, looping on partial writes.
	 * @returns total bytes sent (0 if would block).
	 * @throws on real errors.
	 */
	ssize_t send(std::string_view data) const;

	/**
	 * @brief Receive up to 4KB into `buffer`.
	 * @returns bytes read (>0), 0 if closed or would block.
	 * @throws on real errors.
	 */
	ssize_t receive(std::string& buffer) const;

	/** Underlying socket FD. */
	int getFd() const noexcept { return fd_; }

private:
	int			fd_{-1};
	sockaddr_in	addr_{};
	bool		isListening_{false};

	void throwIfInvalid(const char* what) const;
};
