
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <iostream>
 
#include "server.hpp"

/* Cette macro permet à tout ceux qui ont la flemme de réécrire
 * la vérification des fonctions POSIX de n'écrire qu'une seule ligne */
#define ERROR_IF(FUNCTION) \
    if (FUNCTION) { perror(#FUNCTION); exit(errno); }

void net::Server::connect()
{
    struct sockaddr_in addr;

    /* Création de la socket */
    ERROR_IF((_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1);
 
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
 
    ERROR_IF((newfd = ::accept(_fd, &client, &clientlen)) == -1);

	auto clientPair = _clients.insert(Client(newfd));
	
	if (!clientPair.second)
		std::cerr << "Erreur d'insertion d'un client." << std::endl;
 
    return *clientPair.first;
}
