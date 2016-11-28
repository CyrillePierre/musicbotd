
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <cstdio>
#include <thread>
#include <mutex>
#include <unistd.h>

namespace net
{

/**
 * Cette classe s'occupe de la gestion de chaque client qui se connecte
 * au serveur.
 * @author Cyrille Pierre
 */
class Client
{
	int _fd;	// Le file descriptor de la socket
    mutable std::mutex _mutex;

public:
	/** @brief Constructeur */
    Client(int fd) : _fd(fd) {}
    Client(Client const & c) = delete;
    Client(Client && c) : _fd(c._fd) { c._fd = -1; }
    ~Client() { close(); }

	/** @brief Exécution de la gestion du client */
	template <class Callable>
	void asyncRun(Callable && callable) const;

	/** @brief Comparateur */
	inline bool operator<(Client const & c) const { return _fd < c._fd; }

	/** @brief Fermeture de la socket du client */
    inline void close() const { if (_fd != -1) ::close(_fd); }

	/** @brief Envoi de donnée au client. */
	template <typename Buffer>
	int write(Buffer && buf, std::size_t size) const;

	/** @brief Réception de données du client. */
	template <typename Buffer>
	int read(Buffer && buf, std::size_t size) const;
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

/**
 * Cette fonction est bloquante tant que les données n'ont pas été envoyées.
 * @param buf  : le buffer à envoyer chez le client.
 * @param size : la taille du buffer.
 * @return le nombre d'octets écrits ou -1 si une erreur s'est produite.
 */
template <typename Buffer>
inline int net::Client::write(Buffer && buf, std::size_t size) const {
    std::lock_guard<std::mutex> lock{_mutex};
	return ::write(_fd, std::forward<Buffer>(buf), size);
}

/**
 * Cette fonction est bloquante tant qu'il n'y a pas de données à lire.
 * @param buf  : le buffer contenant les données lues.
 * @param size : le nombre d'octets à lire.
 * @return le nombre d'octets lues ou -1 si une erreur s'est produite.
 */
template <typename Buffer>
inline int net::Client::read(Buffer && buf, std::size_t size) const {
    return ::read(_fd, std::forward<Buffer>(buf), size);
}

#endif
