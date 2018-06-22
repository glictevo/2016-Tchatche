#ifndef TCHATCHE_H
# define TCHATCHE_H

# define S "/tmp/pipe_s"
# include <string.h>
# include <stdio.h>
# include <fcntl.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <unistd.h>
# include <stdlib.h>
# include <errno.h>
# include <sys/select.h>

# define MAX(x,y) ((x)>(y)?(x):(y))
# define BUFFER_SIZE 1024 //Peut-être que le suejt dit de mettre une autre valeur ?

/*
Cette structure définit ce qu'est un utilisateur selon le serveur
- id : c'est le numéro d'utilisateur attribué par le serveur
- name : c'est le nom d'utilisateur choisi par celui-ci
- tube : c'est le tube dédié à l'utilisateur, ouvert en lecture dans la partie serveur
*/
typedef struct _user_lst {
  int               id;
  char              *name;
  int               tube; //On changerait pas le nom pour fd ? Enfin si c'est bien à ça que ça correspond, j'ai un doute...
  struct _user_lst  *next;
} user_lst;

/*
  Cette structure sert à faire une liste des transferts en cours
  - fd_destinataire : le file descriptor du destinataire, du côté client, il s'agit du fd du fichier en cours d'écriture
  - id_transfert : l'id du transfert en cours
  - next : le prochain transfert de la liste
*/
typedef struct  _trsf_lst
{
  char              *file_name;
  int               fd_destinataire;
  int               id_transfert;
  int               nb_msg;
  struct _trsf_lst  *next;
} trsf_lst;

//Liste des fonctions (ordonnées à peu près)
//TODO : ajouter les fonctions non écrites
int       lst_length(user_lst **list);
void      add_user(user_lst **list, user_lst *new);
user_lst  *create_user(char *pseudo, int fd);
void      print_user_list(user_lst **list);
void      remove_user(user_lst **list, int id);
void      delete_list_user(user_lst **list);
user_lst  *find_user_id(user_lst **list, int id);
user_lst  *find_user_pseudo(user_lst **list, char *pseudo);

void      add_trsf(trsf_lst **list, trsf_lst *new);
trsf_lst  *create_trsf(int fd_destinataire, int nb_msg, char* file_name);
trsf_lst  *create_trsf2(int fd_destinataire, int nb_msg, char *file_name, int id_transfert);
void      print_trsf_list(trsf_lst **list);
void      remove_trsf(trsf_lst **list, int id_transfert);
void      delete_list_trsf(trsf_lst **list);
trsf_lst  *find_trsf_id(trsf_lst **list, int id_transfert);

void      connection(char *message, user_lst **users, int index);
void      disconnection(char *message, user_lst **users, int index);
void      send_public_message(char *message, user_lst **users, int index);
void      send_private_message(char *message, user_lst **users, int index);
int       analyze_msg(char *message, user_lst **users);

void      send_private_message_client(char *input, int fd_s, int id_client, int fd_c);
void      disconnection_client(int fd_s, int id_client);
void      analyze_command(char *input, int fd_s, int id_client, int fd_c);
void      analyze_input(char *input, int fd_s, int id_client, int fd_c);
int       analyse_server_msg(char *buffer, trsf_lst **trsf);

void      display_bcst(char *message);
void      display_prvt(char *message);

void      add_int_at(int n, char *str);
int       add_str_at(char *to_add, char *str);
char      *get_substring(char *str, int n);
char      *get_substring_until(char* str, char c);
char      *get_substring_from_end_until(char  *str, char c);
int       get_int(char *str);
int       get_longint(char *str);

char      *create_msg(char *pseudo, char *message, int mode);
char      *create_list_msg(int n, char *pseudo);
char      *create_client_pv_msg(int id, char *pseudo, char *message);
char      *create_client_bcst_msg(int id, char *message);
char      *create_send_msg(int id_client, char *pseudo, int size_file, char *file_name);
char      *create_send_msg_first_step(int id_transfert, int size_file, char *file_name);
void      send_notification(user_lst **users, char *pseudo, int mode);


#endif
