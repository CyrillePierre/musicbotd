#include "client.hpp"


/**
 * Cette fonction est bloquante tant que les données n'ont pas été envoyées.
 * @param buf  : le buffer à envoyer chez le client.
 * @param size : la taille du buffer.
 * @return le nombre d'octets écrits ou -1 si une erreur s'est produite.
 */
int net::Client::write(void const * buf, std::size_t size) const {
    std::lock_guard<std::mutex> lock{_mutex};
	return ::write(_fd, buf, size);
}


/**
 * Cette fonction est bloquante tant qu'il n'y a pas de données à lire.
 * @param buf  : le buffer contenant les données lues.
 * @param size : le nombre d'octets à lire.
 * @return le nombre d'octets lues ou -1 si une erreur s'est produite.
 */
int net::Client::read(void * buf, std::size_t size) const {
	fd_set fds;
	timeval time{_timeout, 0};
	FD_ZERO(&fds);
	FD_SET(_fd, &fds);
	int res = select(_fd+1, &fds, nullptr, nullptr, &time);
	if (res == -1) return -1;
	if (res) return ::read(_fd, buf, size);
    return 0;
}
