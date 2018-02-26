
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

extern "C" {
#include "signal.h"
}

#include "server.hpp"


static void sigpipe(int) {}


/* Cette macro permet à tout ceux qui ont la flemme de réécrire
 * la vérification des fonctions POSIX de n'écrire qu'une seule ligne */
#define ERROR_IF(FUNCTION) \
    if (FUNCTION) { perror(#FUNCTION); exit(errno); }

net::Server::Server(int port, long timeout):
	_timeout{timeout},
	_port(port),
	_threadAccept{false, std::thread{}} {
	signal(SIGPIPE, &sigpipe);
}

net::Server::~Server() {
	if(std::get<0>(_threadAccept)) std::get<1>(_threadAccept).join();
}

void net::Server::connect()
{
    int tr = 1;
    struct sockaddr_in addr;

    /* Création de la socket */
    ERROR_IF((_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1);

    /* Avoid "address already in use" */
    ERROR_IF(setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &tr, sizeof(int)) == -1);
		ERROR_IF(fcntl(_fd, F_SETFL, O_NONBLOCK) == -1);
 
    /* Initialisation de l'adresse */
    addr.sin_family		 = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port		 = htons(_port);
 
    /* On attache la socket à l'interface */
    ERROR_IF(bind(_fd, (sockaddr *) &addr, sizeof(struct sockaddr_in)) == -1);
 
    /* On règle le nombre de connexion simultanée */
    ERROR_IF(listen(_fd, 5) == -1);
}

void net::Server::disconnect()
{
    std::lock_guard<std::mutex> lock{_clientListMutex};
    for (Client const & c : _clients) c.close();
	close(_fd);
    _clients.clear();
}

/**
 * Cette mhétode permet d'attendre la connexion d'un client.
 * @return le client qui vient de se connecter.
 */
net::Client const & net::Server::accept()
{
    struct sockaddr client;
    socklen_t       clientlen = sizeof(client);
    int             newfd;
 
	newfd = ::accept(_fd, &client, &clientlen);
	if(newfd == -1) {
		if(errno == EAGAIN || errno == EWOULDBLOCK) throw std::runtime_error{"error: would block"};
		perror("newfd == -1");
		exit(errno);
	}

    _clientListMutex.lock();
	auto clientPair = _clients.insert(Client(newfd, _timeout));
    _clientListMutex.unlock();
	
	if (!clientPair.second)
		std::cerr << "Erreur d'insertion d'un client." << std::endl;
 
    return *clientPair.first;
}
