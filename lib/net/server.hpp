#ifndef SERVER_HPP
#define SERVER_HPP

#include <set>
#include <thread>
#include <iostream>
#include "client.hpp"

namespace net
{

/**
 * Cette classe s'occupe du réseau. Elle permet de mettre en place
 * un serveur TCP sur le port 1337. Dès qu'un client se connectera
 * on lui enverra les données des capteurs.
 *
 * Le serveur n'a pas besoin d'accepter plusieurs utilisateurs mais
 * ça pourra être modifié par la suite.
 * @author Cyrille Pierre
 */
struct Server
{
	typedef std::set<Client> Clients;

private:
	int		_port;		// port de connexion
	int		_fd;		// socket
	Clients	_clients;	// Ensemble des clients connectés

public:
	/** @brief Constructeur */
	Server(int port) : _port(port) {}

	/** @brief Démarrage du serveur */
	void connect();

	/** @brief Déconnexion du serveur */
	void disconnect();

	/** @brief Attente de connexion d'un client */
	Client const & accept();

	/** @brief Attente asynchrone de connexion */
	template <typename Callable>
	void asyncAcceptLoop(Callable && fn);

	/** @brief Ecriture à tous les clients */
	template <typename Buffer>
	void writeAll(Buffer && buf, std::size_t size) const;
};

} // end namespace net


/**
 * Cette méthode s'occupe de reçevoir les clients mais contrairement
 * à la méthode accept, l'attente est faite en boucle en arrière plan.
 * Lorsqu'un client se connectera, il exécutera la fonction passée en paramètre.
 * Elle doit être de la forme : void (*)(Client const &).
 * @param fn : foncteur appelé à la connexion d'un client.
 */
template <typename Callable>
void net::Server::asyncAcceptLoop(Callable && fn)
{
	auto clientFn = [&fn, this] (net::Client const & c) {
		// Exécution de la tâche confié au client
		std::forward<Callable>(fn)(c);

		// Quand le client s'est déconnecté, on le retire de la liste
		_clients.erase(c);
	};

	// Ce thread va exécuter en boucle la méthode accept()
	std::thread([clientFn, this] () { 
		while (1) {
			accept().asyncRun(clientFn);
		}
	}).detach();
}

/**
 * Envoi d'un buffer simultanément à tous les clients qui sont actuellement
 * connecté au serveur.
 * @param buf  : le buffer contenant les données à envoyer aux clients.
 * @param size : la taille du buffer
 */
template <typename Buffer>
inline void net::Server::writeAll(Buffer && buf, std::size_t size) const
{
	for (net::Client const & c : _clients)
		c.write(std::forward<Buffer>(buf), size);
}

#endif
