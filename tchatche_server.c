//TODO : vérifier que tous les malloc sont bien free

#include "tchatche.h"

void connection(char *message, user_lst **users, int index)
{
  user_lst  *new_user;
  char  *pseudo;
  char  *tube;
  char  *to_send;
  int   fd;

  //On récupère le pseudo
  pseudo = get_substring(&message[index + 4], get_int(&message[index]));
  index += 4 + strlen(pseudo);

  //On récupère le tube
  tube = get_substring(&message[index + 4], get_int(&message[index]));
  index += 4 + strlen(pseudo);

  //On récupère le file descriptor du tube
  fd = open(tube, O_WRONLY);
  if (fd == -1)
    return ;

  //On vérifie que le pseudo n'existe pas
  if (find_user_pseudo(users, pseudo) != NULL)
  {
    //Envoi de message de BADD : pseudo déjà utilisé
    write(fd, "0008BADD\0", 9);
    close(fd);
    free(tube);
    free(pseudo);
    return ;
  }

  //On créé le nouvel utilisateur
  if ((new_user = create_user(pseudo, fd)) == NULL)
  {
    //Envoie de message BADD : fail à la création de l'user
    write(fd, "0008BADD\0", 9);
    close(fd);
    free(tube);
    free(pseudo);
    return ;
  }

  //On ajoute le nouvel utilisateur à la liste des utilisateurs
  add_user(users, new_user);

  //NOTE : j'affiche la liste des utilisateurs courants
  print_user_list(users);

  //Création du message OKOK
  to_send = (char *)malloc(sizeof(char) * 13);
  strcpy(to_send, "0012OKOK");
  add_int_at(new_user->id, &to_send[8]);
  to_send[12] = '\0';

  //Envoi du message
  write(fd, to_send, strlen(to_send));

  //TODO : notifier tous les utilisateurs
  send_notification(users, pseudo, 1);

  free(pseudo);
  free(tube);
  free(to_send);
}

/*
Cette fonction est appelée après qu'un utilisateur ait sollicité une déconnexion.
Dans ce cas, la fonction envoie un message à l'utilisateur puis
détruit les éléments relatifs à cet utilisateur (tube, nom, id).
NOTE : vérifier que ça fonctionne
*/
void disconnection (char *message, user_lst **users, int index) {
  int       id;
  int       fd;
  user_lst  *user;

  id = get_int(&message[index]);
  user = find_user_id(users, id);

  if (user != NULL)
  {
    fd = user->tube;

    //TODO : notifier tous les utilisateurs
    send_notification(users, user->name, 0);
    printf("J'ai notifié tout le monde\n");
    remove_user(users, id);
    write(fd, message, strlen(message));
    close(fd);
  }
}

void send_public_message(char *message, user_lst **users, int index)
{
  user_lst  *tmp_user;
  char      *user_message;
  char      *user_pseudo;
  char      *to_send;
  int       id;

  //On recupère l'id de celui qui a envoyé le message
  id = get_int(&message[index]);
  index += 4;

  //On récupère son message
  user_message = get_substring(&message[index + 4], get_int(&message[index]));

  //On récupère le pseudo de l'envoyeur
  tmp_user = find_user_id(users, id);
  user_pseudo = tmp_user->name;

  //On créé le message a envoyer
  to_send = create_msg(user_pseudo, user_message, 1);

  //On envoie le message à tous les utilisateurs
  tmp_user = *users;
  while (tmp_user)
  {
    ////printf("J'envoie le bcst à : %s\n", tmp_user->name);
    int fd = tmp_user->tube;
    write(fd, to_send, strlen(to_send));
    tmp_user = tmp_user->next;
  }

  free(user_message);
  free(to_send);
}

void send_private_message(char *message, user_lst **users, int index)
{
  user_lst  *tmp_user;
  char      *user_message;
  char      *user_pseudo;
  char      *to_send_pseudo;
  char      *to_send;
  int       id;
  int       to_send_fd;
  int       user_fd;

  //On recupère l'id de celui qui a envoyé le message
  id = get_int(&message[index]);
  index += 4;

  //On récupère le pseudo de l'envoyeur
  tmp_user = find_user_id(users, id);
  user_pseudo = tmp_user->name;

  //On récupère le file descriptor de l'envoyeur
  user_fd = (find_user_pseudo(users, user_pseudo))->tube;

  //On récupère le pseudo du destinataire
  to_send_pseudo = get_substring(&message[index + 4], get_int(&message[index]));
  index += 4 + strlen(to_send_pseudo);
  printf("to_send_pseudo : %s\n", to_send_pseudo);

  //On vérifie que le destinataire existe bien, sinon envoi de BADD
  if (find_user_pseudo(users, to_send_pseudo) == NULL)
  {
    printf("J'envoie BADD\n");
    write(user_fd, "0008BADD\0", 9);
    free(to_send_pseudo);
    return ;
  }

  //On récupère son message
  user_message = get_substring(&message[index + 4], get_int(&message[index]));
  printf("user_message : %s\n", user_message);

  //On créé le message a envoyer
  to_send = create_msg(user_pseudo, user_message, 0);

  //On récupère le file descriptor du destinataire
  tmp_user = find_user_pseudo(users, to_send_pseudo);
  to_send_fd = tmp_user->tube;

  //On envoie le message au destinataire
  write(to_send_fd, to_send, strlen(to_send));

  //On envoi un OKOK à l'envoyeur
  write(user_fd, "0008OKOK\0", 9);
  printf("J'envoie OKOK au fd : %d\n", user_fd);

  free(user_message);
  free(to_send_pseudo);
  free(to_send);
}

void shut_server(user_lst **users)
{
  char      *to_send;
  user_lst  *tmp;

  //Création du début du message
  to_send = (char *)malloc(sizeof(char) * 13);
  strcpy(to_send, "0012SHUT");
  to_send[12] = '\0';

  //Changement de la fin du message et envoie du message aux utilisateurs
  tmp = *users;
  while(tmp)
  {
    add_int_at(tmp->id, &to_send[8]);
    write(tmp->tube, to_send, 12);
    tmp = tmp->next;
  }

  free(to_send);
}

/*
  Envoie la liste des utilisateurs à l'utilisateur qui le veut
*/
void send_user_list(char *message, user_lst **users, int index)
{
  user_lst  *tmp_user;
  char      *to_send;
  int       to_send_fd;
  int       nb_users;

  //On récupère le file descriptor de l'utilisateur à qui on envoie la liste
  tmp_user = find_user_id(users, get_int(&message[index]));
  to_send_fd = tmp_user->tube;

  //On envoie un message privé au client pour lui dire qu'on va afficher la liste des utilisateurs connectés
  write(to_send_fd, "0057PRVT0006server0035Liste des utilisateurs connectés : ", 57);

  //On recherche combien d'utilisateurs sont connectés
  nb_users = lst_length(users);

  //Pour chaque utilisateur connecté on envoie un message
  tmp_user = *users;
  while(tmp_user)
  {
    to_send = create_list_msg(nb_users, tmp_user->name);
    write(to_send_fd, to_send, strlen(to_send));

    tmp_user = tmp_user->next;
    free(to_send);
  }
}

void send_file_first_step(char *message, user_lst **users, trsf_lst **trsf)
{
  user_lst  *tmp_user;
  trsf_lst  *tmp_trsf;
  char  *to_send_pseudo;
  char  *to_send;
  char  *file_name;
  int   size_file;
  int   user_id;
  int   user_fd;
  int   to_send_fd;

  //On récupère l'id de l'envoyeur
  user_id = get_int(message);

  //On récupère le file descriptor de l'envoyeur
  tmp_user = find_user_id(users, user_id);
  user_fd = tmp_user->tube;

  //On récupère le pseudo du destinataire
  to_send_pseudo = get_substring(&message[8], get_int(&message[4]));

  //On regarde si le destinataire existe et si oui on choppe son fd
  if ((tmp_user = find_user_pseudo(users, to_send_pseudo)) == NULL)
  {
    write(user_fd, "0008BADD\0", 9);
    free(to_send_pseudo);
    return ;
  }
  to_send_fd = tmp_user->tube;

  //On récupère la longueur du fichier
  size_file = get_longint(&message[8 + strlen(to_send_pseudo)]);

  //On récupère le nom du fichier
  file_name = get_substring(&message[16 + strlen(to_send_pseudo)], get_int(&message[12 + strlen(to_send_pseudo)]));
  printf("file_name : %s\n", file_name);

  //On créé la structure de transfert
  tmp_trsf = create_trsf(to_send_fd, size_file / 256 + 1, file_name);

  //On ajoute à la liste des transferts
  add_trsf(trsf, tmp_trsf);

  //On créé le message de réponse pour l'envoyeur
  to_send = (char *)malloc(sizeof(char) * 13);
  add_int_at(12, to_send);
  strcpy(&to_send[4], "OKOK");
  add_int_at(tmp_trsf->id_transfert, &to_send[8]);

  //On envoie le message à l'envoyeur
  write(user_fd, to_send, 13);
  free(to_send);

  //On créé le message pour le destinataire
  to_send = create_send_msg_first_step(tmp_trsf->id_transfert, size_file, file_name);

  //On envoie le message au destinataire
  printf("On envoie le first step\n");
  write(to_send_fd, to_send, strlen(to_send));
  printf("First step envoyé\n");

  free(to_send);
  free(file_name);
  free(to_send_pseudo);
}

void send_file_server(char *message, user_lst **users, int index)
{
  static trsf_lst *trsf;
  trsf_lst        *tmp_trsf;
  char            *new_message;
  int             id_transfert;
  int             serie;

  //On récupère la série
  serie = get_int(&message[index]);

  //Si c'est le message avec comme série 0
  if (serie == 0)
  {
    send_file_first_step(&message[index + 4], users, &trsf);
    return ;
  }

  //sinon si la série est > 0
  //On récupère l'id de transfert
  id_transfert = get_int(&message[index + 4]);

  //On récupère le trsf correspondant
  tmp_trsf = find_trsf_id(&trsf, id_transfert);

  //On créé le new_message
  new_message = (char *)malloc(sizeof(char) * (277));
  strncpy(new_message, message, 276);
  new_message[276] = '\0';

  //On envoie le message au destinataire
  printf("Envoie au destinataure : %s\n", new_message);
  write(tmp_trsf->fd_destinataire, new_message, strlen(new_message));

  //On regarde si c'était le dernier message, et si oui on supprime ce transfert
  printf("1\n");
  if (serie >= tmp_trsf->nb_msg)
    remove_trsf(&trsf, id_transfert);
  printf("2\n");

  free(new_message);
}

/*
Cette fonction analyse le message reçu par un utilisateur selon
les règles définies par le protocole de communication.
*/
int analyze_msg (char *message, user_lst **users) {
  int   total_length;
  char  type[5];
  int   index = 4;

  //On récupère la longueur totale du message
  //index correspond à l'index dans le message où on commencera à récupérer le type
  total_length = get_int(message);
  //printf("total_length : %d\n", total_length);

  //On récupère le type
  strncpy(type, &message[index], 4);
  type[4] = '\0';
  ////printf("type : %s\n", type);
  index += 4;

  if (strcmp(type, "HELO") == 0) {
    connection(message, users, index);
  } else if (strcmp(type, "BYEE") == 0) {
    disconnection(message, users, index);
  } else if (strcmp(type, "BCST") == 0) {
    send_public_message(message, users, index);
  } else if (strcmp(type, "PRVT") == 0) {
    send_private_message(message, users, index);
  } else if (strcmp(type, "LIST") == 0) {
    send_user_list(message, users, index);
  } else if (strcmp(type, "SHUT") == 0) {
    shut_server(users);
    return 1;
  } else if (strcmp(type, "FILE") == 0) {
    send_file_server(message, users, index);
  }
  //printf("Il reste : %s\n", &message[total_length]);
  if (message[total_length] != 0)
    return analyze_msg(&message[total_length], users);
  printf("Finfin.\n");
  return 0;
}

/*
  fonction principale
  contient la boucle infinie pour le programme
  contient la liste entière des utilisateurs
*/
int main (void)
{
  int       fd_s;
  int       compteur = 0;
  char      buffer[BUFFER_SIZE + 1];
  size_t    l;
  user_lst  *users = NULL; //La liste des utilisateurs

  //On créé le tube du serveur
  if (access(S, F_OK) != 0)
    mkfifo(S, 0666);
  fd_s = open(S, O_RDONLY);
  if (fd_s == -1) { perror("ouverture fifo S"); exit (1); }

  //Ouvrir le tube
  while (1)
  {
    compteur++;
    l = read(fd_s, buffer, BUFFER_SIZE);
    if (l > 0)
    {
      buffer[l] = '\0';
      printf("Je reçois dans le buffer : %s\n", buffer);
      if (analyze_msg(buffer, &users) == 1)
        break;
    }
  }

  //On ferme le serveur
  unlink(S);
  delete_list_user(&users);

  return 1;
}
