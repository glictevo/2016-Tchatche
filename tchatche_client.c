#include "tchatche.h"

//TODO : vérifier que tous les malloc sont bien free

/*
  Fonction qui envoie un message privé
  - input : ce que l'utilisateur a écrit
  - fd_s : le file descriptor du pipe du serveur
  - id_client : l'id du client sur le serveur
  - fd_c : le file descriptor du client
*/
void send_private_message_client(char *input, int fd_s, int id_client, int fd_c)
{
  char  *to_send;
  char  *pseudo;
  char  *buffer;
  int   index = 0;

  //On récupère le pseudo
  pseudo = get_substring_until(input, ' ');
  if(pseudo[strlen(pseudo) - 1] == '\n')
  {
    free(pseudo);
    printf("Veuillez écrire un message\n");
    return ;
  }
  index += strlen(pseudo) + 1;

  //On créé le message à envoyer au serveur
  to_send = create_client_pv_msg(id_client, pseudo, &input[index]);

  //On envoie le message
  write(fd_s, to_send, strlen(to_send));

  //Attendre le OKOK ou le BADD
  while (1)
  {
    size_t l;
    buffer = (char *)malloc(sizeof(char) * (BUFFER_SIZE + 1));

    l = read(fd_c, buffer, BUFFER_SIZE);
    buffer[l] = '\0';

    if (l > 0)
    {
      if (strncmp(&buffer[4], "OKOK", 4) == 0)
      {
        printf("[Vous --> %s] : %s\n", pseudo, &input[index]);
        free(buffer);
        break;
      }
      else if (strncmp(&buffer[4], "BADD", 4) == 0)
      {
        printf("Votre message n'a pas été envoyé, le pseudo du destinataire est peut-être érroné\n");
        free(buffer);
        break;
      }
    }
    free(buffer);
  }
  free(pseudo);
  free(to_send);
}

/*
  Envoie un message de deconnexion au serveur
  - fd_s : le file descriptor du pipe du serveur
  - id_client : l'id du client sur le serveur
*/
void disconnection_client(int fd_s, int id_client)
{
  char  *to_send;

  to_send = (char *)malloc(sizeof(char) * 13);
  strcpy(to_send, "0012BYEE");
  add_int_at(id_client, &to_send[8]);
  to_send[12] = '\0';

  write(fd_s, to_send, 12);

  free(to_send);
}

/*
  Envoie un message au serveur pour le shut down
  - fd_s : le file descriptor du pipe du serveur
  - id_client, l'id du client qui shut down le serveur
*/
void shut_client(int fd_s, int id_client)
{
    char  *to_send;

    //Création du message
    to_send = (char *)malloc(sizeof(char) * 13);
    strcpy(to_send, "0012SHUT");
    add_int_at(id_client, &to_send[8]);
    to_send[12] = '\0';

    //Envoie du message
    write(fd_s, to_send, 12);

    free(to_send);
}

/*
  Envoie un message au serveur demandant la liste des utilisateurs
  - fd_s : le file descriptor du pipe du serveur
  - id_client : l'id du client sur le serveur
*/
void ask_list_users(int fd_s, int id_client)
{
  char  *to_send;

  //Création du message
  to_send = (char *)malloc(sizeof(char) * 13);
  strcpy(to_send, "0012LIST");
  add_int_at(id_client, &to_send[8]);
  to_send[12] = '\0';

  //Envoie du message
  write(fd_s, to_send, 12);

  free(to_send);
}

/*
  Envoit les messages contenant les données du fichier
  - int fd_file : le file descriptor du fichier
  -
*/
void send_file_client_2(int fd_file, int id_transfert, int fd_s)
{
  char  *to_send;
  char  buffer[257];
  int   serie = 1;

  //On créé le début du message à envoyer
  to_send = (char *)malloc(sizeof(char) * 277);
  add_int_at(276, to_send);
  strcpy(&to_send[4], "FILE");
  add_int_at(id_transfert, &to_send[12]);
  add_int_at(256, &to_send[16]);
  //La partie changeante du message
  while (read(fd_file, buffer, 256))
  {
    add_int_at(serie, &to_send[8]);
    serie++;
    buffer[256] = '\0';
    strcpy(&to_send[20], buffer);

    //On envoie le message au serveur
    write(fd_s, to_send, strlen(to_send));
  }
  free(to_send);
}

/*
  Envoit le message de demande de transfert de fichier si tout va bien
  - input : l'input de l'utilisateur
  - fd_s : le file descriptor du serveur
  - id_client : l'id du client
  - fd_c : le file descriptor du client
*/
void send_file_client(char *input, int fd_s, int id_client, int fd_c)
{
  char  *to_send;
  char  *file_path;
  char  *file_name;
  char  *pseudo;
  char  *buffer;
  int   fd_file;
  off_t size_file;

  //On récupère le chemin du fichier
  file_path = get_substring_until(input, ' ');

  //On essaie d'ouvrir le fichier
  if ((fd_file = open(file_path, O_RDONLY)) == -1)
  {
    printf("Le fichier n'existe pas\n");
    free(file_path);
    return ;
  }

  //On récupère le pseudo du destinataire
  pseudo = get_substring_until(&input[strlen(file_path) + 1], ' ');

  //On récupère le nom du fichier
  file_name = get_substring_from_end_until(file_path, '/');

  //On récupère la taille du fichier
  size_file = lseek(fd_file, 0, SEEK_END);
  close(fd_file);

  //On créé le message à envoyer au serveur
  to_send = create_send_msg(id_client, pseudo, size_file, file_name);

  //On envoie le message au serveur
  write(fd_s, to_send, strlen(to_send));

  //On lit la réponse du serveur
  //Attendre le OKOK ou le BADD
  while (1)
  {
    size_t l;
    buffer = (char *)malloc(sizeof(char) * (BUFFER_SIZE + 1));

    l = read(fd_c, buffer, BUFFER_SIZE);
    buffer[l] = '\0';

    if (l > 0)
    {
      if (strncmp(&buffer[4], "OKOK", 4) == 0)
      {
        printf("Le fichier va être envoyé\n");
        fd_file = open(file_path, O_RDONLY);
        send_file_client_2(fd_file, get_int(&buffer[8]), fd_s);
        free(buffer);
        break;
      }
      else if (strncmp(&buffer[4], "BADD", 4) == 0)
      {
        printf("Fichier non envoyé, le pseudo du destinataire est peut-être érroné\n");
        free(buffer);
        break;
      }
    }
    free(buffer);
  }
  free(to_send);
  free(pseudo);
  free(file_path);
  free(file_name);

  close(fd_file);
}

/*
  Fonction qui analyse une commande écrite par le client
  Envoie au serveur ce qui est voulu par le client
  si la commande n'existe pas, on affiche un message au client
  - input : ce que l'utilisateur a écrit
  - fd_s : le file descriptor du pipe du serveur
  - id_client : l'id de client sur le serveur
  - fd_c : le file descriptor du client
*/
void analyze_command(char *input, int fd_s, int id_client, int fd_c)
{
  if (strncmp(input, "/pv ", 4) == 0)
    send_private_message_client(&input[4], fd_s, id_client, fd_c);
  else if (strncmp(input, "/deco", 5) == 0)
    disconnection_client(fd_s, id_client);
  else if (strncmp(input, "/shut", 5) == 0)
    shut_client(fd_s, id_client);
  else if (strncmp(input, "/list", 5) == 0)
    ask_list_users(fd_s, id_client);
  else if (strncmp(input, "/send", 5) == 0)
    send_file_client(&input[6], fd_s, id_client, fd_c);
  else
    printf("Commande inconnue\n");
}

/*
  Fonction qui sert à regarder ce que le client a écrit dans son terminal
  Si ça commence par un '/', on considère que c'est une commande et on lance la fonction correspondante
  - input : ce que l'utilisateur a écrit
  - fd_s : le file descriptor du pipe du serveur
  - id_client : l'id du client sur le serveur
  - fd_c : le file descriptor du client
*/
void analyze_input(char *input, int fd_s, int id_client, int fd_c)
{
  int   input_length = strlen(input) - 1;
  int   total_length = 16 + input_length;
  char  *to_send;

  input[input_length] = '\0';
  if (input_length > 0 && input[0] == '/')
  {
    analyze_command(input, fd_s, id_client, fd_c);
    return ;
  }

  //On créé le message a envoyer au serveur
  to_send = create_client_bcst_msg(id_client, input);

  //On envoie le message de l'utilisateur au serveur
  write(fd_s, to_send, total_length);

  free(to_send);
}

/*
  affiche un message public
*/
void display_bcst(char *message)
{
  char  *pseudo;
  int   pseudo_length;

  pseudo_length = get_int(message);

  pseudo = (char *)malloc(sizeof(char) * (pseudo_length + 1));
  pseudo = strncpy(pseudo, &message[4], pseudo_length);
  pseudo[pseudo_length] = '\0';
  write(1, "[", 1);
  write(1, pseudo, pseudo_length);
  write(1, "] : ", 4);
  write(1, &message[pseudo_length + 8], get_int(&message[pseudo_length + 4]));
  write(1, "\n", 1);
  //printf("[%s] : %s\n", pseudo, &message[pseudo_length + 8]);

  free(pseudo);
}

/*
  affiche un message privé
*/
void display_prvt(char *message)
{
  char  *pseudo;
  int   pseudo_length;

  pseudo_length = get_int(message);

  pseudo = (char *)malloc(sizeof(char) * (pseudo_length + 1));
  pseudo = strncpy(pseudo, &message[4], pseudo_length);
  pseudo[pseudo_length] = '\0';
  write(1, "[", 1);
  write(1, pseudo, pseudo_length);
  write(1, " --> Vous] : ", 13);
  write(1, &message[pseudo_length + 8], get_int(&message[pseudo_length + 4]));
  write(1, "\n", 1);
  //printf("[%s --> Vous] : %s\n", pseudo, &message[pseudo_length + 8]);

  free(pseudo);
}

/*
  Affiche le pseudo d'un utilisateurs de la liste des utilisateurs connectés
*/
void display_list(char *message)
{
  write(1, "- ", 2);
  write(1, &message[4], get_int(message));
  write(1, "\n", 1);
}

void recept_file_first_step(char *message, trsf_lst **trsf)
{
  trsf_lst *tmp_trsf;
  char   *file_name;
  int   id_transfert;
  int   size_file;
  int   fd_file;

  //On récupère l'id de transfert
  id_transfert = get_int(message);

  //On récupère la longueur du fichier
  size_file = get_longint(&message[4]);

  //On récupère le nom du fichier
  file_name = get_substring(&message[20], get_int(&message[16]));

  //On ouvre le fichier
  fd_file = open(file_name, O_CREAT|O_WRONLY, 0666);

  //Création d'une liste de transfert
  tmp_trsf = create_trsf2(fd_file, size_file / 256 + 1, file_name, id_transfert);

  //On l'ajoute à la liste des transferts
  add_trsf(trsf, tmp_trsf);

  free(file_name);
}

void recept_file(char *message, trsf_lst **trsf)
{
  trsf_lst  *tmp_trsf;
  char      data[257];
  int       serie;
  int       id_transfert;
  int       fd_file;

  //On récupère la série
  serie = get_int(message);

  //Si la série est à 0
  if (serie == 0)
  {
    recept_file_first_step(&message[4], trsf);
    return ;
  }

  //On récupère l'id de transfert
  id_transfert = get_int(&message[4]);

  //On récupère le transfert correspondant
  tmp_trsf = find_trsf_id(trsf, id_transfert);

  //On récupère le file descriptor du fichier
  fd_file = tmp_trsf->fd_destinataire;

  //On récupère les donnnées
  strncpy(data, &message[12], 256);
  data[256] = '\0';

  //On écrit dans le fichier
  write(fd_file, data, 257);

  //Si on a fini de tout écrire, on close le fichier
  if (serie >= tmp_trsf->nb_msg)
  {
    printf("Vous avez reçu le fichier %s\n", tmp_trsf->file_name);
    remove_trsf(trsf, id_transfert);
    close(fd_file);
  }
}

/*
  Analyse ce qu'on lit sur le fd du serveur
  Renvoie 1 si on fait une deconnexion
  - trsf : la liste des transferts en cours
*/
int analyse_server_msg(char *buffer, trsf_lst **trsf)
{
  int   total_length = get_int(buffer);

  if (strncmp(&buffer[4], "BCST", 4) == 0)
    display_bcst(&buffer[8]);
  else if (strncmp(&buffer[4], "PRVT", 4) == 0)
    display_prvt(&buffer[8]);
  else if (strncmp(&buffer[4], "BYEE", 4) == 0)
    return 1;
  else if (strncmp(&buffer[4], "LIST", 4) == 0)
    display_list(&buffer[12]);
  else if (strncmp(&buffer[4], "SHUT", 4) == 0)
  {
    printf("Le serveur ferme, byee\n");
    return 1;
  }
  else if (strncmp(&buffer[4], "FILE", 4) == 0)
    recept_file(&buffer[8], trsf);

  if (buffer[total_length] != '\0')
    return analyse_server_msg(&buffer[total_length], trsf);
  return 0;
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Veuillez lancer le programme avec un argument votre pseudo");
    exit(1);
  }

  if (strcmp("server", argv[1]) == 0)
  {
    printf("Vous ne pouvez pas prendre le pseudo : server\n");
    exit(1);
  }

  if (strlen(argv[1]) > 50)
  {
    printf("Votre pseudo ne doit pas dépasser 50 caractères\n");
    exit(1);
  }

  if (strstr(" ", argv[1]))
  {
    printf("Vous ne pouvez pas avoir d'espaces dans votre pseudo\n");
    exit(1);
  }

  trsf_lst  *trsf;
  char  *chemin;
  char  *to_send;
  char  buffer[BUFFER_SIZE + 1];
  char  *type;
  int   fd_c;
  int   total_length;
  int   fd_s;
  int   id_client;
  size_t  l;

  //On créé le chemin vers le pipe
  chemin = (char *)malloc(sizeof(char) * (strlen(argv[1]) + 6));
  strcpy(chemin, "/tmp/");
  strcat(chemin, argv[1]);
  chemin[strlen(chemin)] = '\0';

  //Créer le tube avec le pseudo
  if (access(chemin, F_OK) != 0)
    mkfifo(chemin, 0666);
  fd_c = open(chemin, O_RDONLY|O_NONBLOCK);
  if (fd_c == -1) { perror("ouverture fifo client"); exit (1); }

  //On récupère le fd du pipe du serveur
  fd_s = open(S, O_WRONLY);

  //Envoyer un message au serveur
  total_length = 16 + strlen(argv[1]) + strlen(chemin);
  to_send = (char *)malloc(sizeof(char) * (total_length + 1));
  add_int_at(total_length, to_send);
  strcpy(&to_send[4], "HELO");
  add_str_at(argv[1], &to_send[8]);
  add_str_at(chemin, &to_send[12 + strlen(argv[1])]);
  to_send[total_length] = '\0';

  write(fd_s, to_send, total_length);

  //On recupère la réponse du serveur
  while(1)
  {
    //Si c'est bon on continue
    //Sinon on quitte le programme
    l = read(fd_c, buffer, BUFFER_SIZE);
    if (l > 0)
    {
      buffer[l] = '\0';

      //On récupère le type (OKOK ou BADD)
      type = get_substring(&buffer[4], 4);

      if (strcmp(type, "BADD") == 0)
      {
        printf("Erreur : pseudo déjà utilisé ou autre\n");
        exit(1);
      }
      else if (strcmp(type, "OKOK") == 0)
      {
        //On récupère l'id du client donné par le serveur
        id_client = get_int(&buffer[8]);
        free(type);
        break;
      }
      /*else
      {
        printf("Erreur inopinée : exit\n");
        exit(1);
      }*/
      free(type);
    }
  }

  fd_set  enslec;
  int result;
  while(1)
  {
    FD_ZERO(&enslec);
    FD_SET(STDIN_FILENO, &enslec);
    FD_SET(fd_c, &enslec);

    result = select(fd_c + 1, &enslec, NULL, NULL, NULL);
    if (result == -1 && errno != EINTR)
    {
      printf("erreur\n");
      break;
    }
    else if (result == -1 && errno == EINTR)
    {
      printf("deuxième cas\n");
    }
    else
    {
      if (FD_ISSET(STDIN_FILENO, &enslec))
      {
        //Récupérer l'entrée standard
        l = read(0, buffer, BUFFER_SIZE);
        if (l > 0)
        {
          buffer[l] = '\0';
          analyze_input(buffer, fd_s, id_client, fd_c);
        }
      }

      if(FD_ISSET(fd_c, &enslec))
      {
        //Lire sur fd_c
        l = read(fd_c, buffer, BUFFER_SIZE);
        if (l > 0)
        {
          buffer[l] = '\0';
          //Analyser ce que le serveur a envoyé
          if (analyse_server_msg(buffer, &trsf) == 1)
            break;
        }
      }
    }
  }

  close(fd_c);
  unlink(chemin);
  free(chemin);
  free(to_send);
  //NOTE : Voir si d'autres trucs à free

  return 1;
}
