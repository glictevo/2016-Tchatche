# 2016-Tchatche
Système de communication interactif multi-utilisateurs local avec des pipes, en C

Projet réalisé par : Timon De Morel et Guillaume Lictevout

Dans le cadre de notre 3ème année de licence informatique à Paris 7 Diderot

## Instructions

* Dans un terminal, à la racine, lancez la commande : make

* Lancez un serveur avec la commande : ./serveur

* Dans un autre terminal, vous pouvez créer un nouveau client avec la commande : ./client [pseudo]

### Les commandes

* [message] : envoyer un message à tous les clients connectés au serveur

* Envoyer un message privé : /pv [pseudo] [message]

* Se déconnecter : /deco

* Fermer le serveur : /shut

* Envoyer un fichier : /send [chemin_fichier] [pseudo]
