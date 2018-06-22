#include "tchatche.h"

/*
  Ajoute 4 caractères pour écrire l'int dans la chaine
  - n : l'int a écrire dans la chaine
  - str : la chaine directement à l'adresse où on veut le int
*/
void add_int_at(int n, char *str)
{
  int   powerof10 = 1000;
  int   i;

  for(i = 0; i < 4; i++)
  {
    str[i] = n / powerof10 % 10 + '0';
    powerof10 /= 10;
  }
}

/*
  Ajoute 8 caractères pour écrire l'int de la chaîne
  - n : l'int à écrire dans la chaîne
  - str : la chaine directement à l'adresse où on veut le int
*/
void add_longint_at(int n, char *str)
{
  int   powerof10 = 10000000;
  int   i;

  for(i = 0; i < 8; i++)
  {
    str[i] = n / powerof10 % 10 + '0';
    powerof10 /= 10;
  }
}

/*
  Ajoute une chaine de caractère et sa longueur juste avant dans la chaîne
  Fini par mettre un \0 à la fin
  - to_add : la chaine dont on veut ajouter à l'autre chaîne
  - str : la chaine dans laquelle on veut ajouter
  @return : la longueur totale de ce qui a été écrit sur la chaîne
*/
int   add_str_at(char *to_add, char *str)
{
  int   length;

  length = strlen(to_add);
  add_int_at(length, str);
  strncpy(&str[4], to_add, length);
  str[length + 4] = '\0';

  return length + 4;
}

/*
  Renvoit le substring à partir de l'adresse donné en argument et sur n octets
  - message : la chaine d'où on extrait
  - n : le nombre de caractères à extraire
*/
char  *get_substring(char *str, int n)
{
  char  *substr;

  substr = (char *)malloc(sizeof(char) * (n+1));
  strncpy(substr, str, n);
  substr[n] = '\0';

  return substr;
}

/*
  Renvoit le substring à partir de l'adresse donnée en argument jusqu'à ce qu'on tombe sur le caractre donné en argument
  - str : la chaine d'où on extrait
  - c : le caractère d'arrêt d'extraction
*/
char  *get_substring_until(char* str, char c)
{
  int   length;

  length = 0;
  while(str[length] && str[length] != c)
    length++;

  return get_substring(str, length);
}

char  *get_substring_from_end_until(char *str, char c)
{
  int   length = 0;
  int   index;

  index = strlen(str);
  while(str[--index] && str[index] != c)
    length++;

  return get_substring(&str[index + 1], length);
}

/*
  Renvoit l'int de la chaine de caractère de manière sécurisée
  (Ne prend sur le atoi que les 4 premiers caractères)
  - str : la chaine d'où extraire l'int
*/
int get_int(char *str)
{
  char  *str_n;
  int   n;

  str_n = get_substring(str, 4);
  n = atoi(str_n);
  free(str_n);

  return n;
}

/*
  Renvoit l'int de la chaine de caractère de manière sécurisée
  (Ne prend sur le atoi que les 8 premiers caractères)
  - str : la chaine d'où extraire l'int
*/
int get_longint(char *str)
{
  char  *str_n;
  int   n;

  str_n = get_substring(str, 8);
  n = atoi(str_n);
  free(str_n);

  return n;
}

/*
  Créé un message de type BCST (message public) ou PRVT (privé)
  - pseudo : le pseudo de l'envoyeur
  - message : son message
  - mode : 1 pour message public
           0 pour message privé
*/
char  *create_msg(char *pseudo, char *message, int mode)
{
  char  *to_return;
  int   total_length;

  total_length = 16 + strlen(pseudo) + strlen(message);
  to_return = (char *)malloc(sizeof(char) * (total_length + 1));

  add_int_at(total_length, to_return);
  if(mode == 1)
    strcpy(&to_return[4], "BCST");
  else
    strcpy(&to_return[4], "PRVT");
  add_str_at(pseudo, &to_return[8]);
  add_str_at(message, &to_return[12 + strlen(pseudo)]);

  return to_return;
}

char  *create_list_msg(int n, char *pseudo)
{
  char   *to_return;
  int   total_length;

  total_length = 16 + strlen(pseudo);
  to_return = (char *)malloc(sizeof(char) * (total_length + 1));

  add_int_at(total_length, to_return);
  strcpy(&to_return[4], "LIST");
  add_int_at(n, &to_return[8]);
  add_str_at(pseudo, &to_return[12]);

  return to_return;
}

char  *create_client_pv_msg(int id, char *pseudo, char *message)
{
  char  *to_return;
  int   total_length;

  total_length = 20 + strlen(pseudo) + strlen(message);
  to_return = (char *)malloc(sizeof(char) * (total_length + 1));

  add_int_at(total_length, to_return);
  strcpy(&to_return[4], "PRVT");
  add_int_at(id, &to_return[8]);
  add_str_at(pseudo, &to_return[12]);
  add_str_at(message, &to_return[16 + strlen(pseudo)]);

  return to_return;
}

char  *create_client_bcst_msg(int id, char *message)
{
  char  *to_return;
  int   total_length;

  total_length = 16 + strlen(message);
  to_return = (char *)malloc(sizeof(char) * (total_length + 1));

  add_int_at(total_length, to_return);
  strcpy(&to_return[4], "BCST");
  add_int_at(id, &to_return[8]);
  add_str_at(message, &to_return[12]);

  return to_return;
}

void send_notification(user_lst **users, char *pseudo, int mode)
{
  user_lst  *tmp_user;
  char  *to_send;
  char  *message;

  message = (char *)malloc(sizeof(char) * (strlen(pseudo) + 20));
  strcpy(message, pseudo);
  if (mode == 1)
    strcpy(&message[strlen(pseudo)], " s'est connecte\0");
  else
    strcpy(&message[strlen(pseudo)], " s'est déconnecte\0");

  to_send = create_msg("server", message, 1);

  tmp_user = *users;
  while(tmp_user)
  {
    write(tmp_user->tube, to_send, strlen(to_send));
    tmp_user = tmp_user->next;
  }

  free(message);
  free(to_send);
}

char *create_send_msg(int id_client, char *pseudo, int size_file, char *file_name)
{
  char  *to_return;
  int   total_length;

  total_length = 32 + strlen(pseudo) + strlen(file_name);
  to_return = (char *)malloc(sizeof(char) * (total_length + 1));

  add_int_at(total_length, to_return);
  strcpy(&to_return[4], "FILE");
  add_int_at(0, &to_return[8]);
  add_int_at(id_client, &to_return[12]);
  add_str_at(pseudo, &to_return[16]);
  add_longint_at(size_file, &to_return[20] + strlen(pseudo));
  add_str_at(file_name, &to_return[28] + strlen(pseudo));

  return to_return;
}

char *create_send_msg_first_step(int id_transfert, int size_file, char *file_name)
{
  char  *to_return;
  int   total_length;

  total_length = 28 + strlen(file_name);
  to_return = (char *)malloc(sizeof(char) * (total_length + 1));

  add_int_at(total_length, to_return);
  strcpy(&to_return[4], "FILE");
  add_int_at(0, &to_return[8]);
  add_int_at(id_transfert, &to_return[12]);
  add_longint_at(size_file, &to_return[16]);
  add_str_at(file_name, &to_return[24]);

  return to_return;
}
