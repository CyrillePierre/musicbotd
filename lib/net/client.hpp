
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <cstdio>
#include <thread>
#include <mutex>
#include <unistd.h>

namespace net {

/**
 * Cette classe s'occupe de la gestion de chaque client qui se connecte
 * au serveur.
 * @author Cyrille Pierre
 */
class Client {
	long const _timeout;
	int _fd;	// Le file descriptor de la socket
	mutable std::string _data;
    mutable std::mutex _mutex;

public:
	/** @brief Constructeur */
    Client(int fd, long timeout) : _timeout{timeout}, _fd(fd) {}
    Client(Client const & c) = delete;
    Client(Client && c) : _timeout{c._timeout}, _fd(c._fd) { c._fd = -1; }
    ~Client() { close(); }

	/** @brief Exécution de la gestion du client */
	template <class Callable>
	void asyncRun(Callable && callable) const;

	/** @brief Comparateur */
	inline bool operator<(Client const & c) const { return _fd < c._fd; }

	/** @brief Fermeture de la socket du client */
    inline void close() const { if (_fd != -1) ::close(_fd); }

	/** @brief Envoi de donnée au client. */
	int write(void const * buf, std::size_t size) const;

	/** @brief Réception de données du client. */
	int read(void * buf, std::size_t size) const;

	/** @brief Réception de données du client jusqu'à un délimiteur. */
	int readUntil(std::string & buf, char d) const;

    int id() const { return _fd; }
};

} // end namespace net


/**
 * Cette méthode permet de préciser en dehors de la classe l'action à
 * faire pour gérer le client. Pour cela il faut lui passer en paramètre
 * une fonction ou un foncteur de la forme void (*)(Client *).
 * @param c : le foncteur
 */
template <class Callable>
inline void net::Client::asyncRun(Callable && callable) const {
	std::thread(std::forward<Callable>(callable), std::cref(*this)).detach();
}


#endif
