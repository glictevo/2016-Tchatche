#include "tchatche.h"

/*
  Cette fonction permet d'ajouter un transfert a une liste de transferts
  - list : la liste des transferts
  - new : le nouveau transfert
*/
void add_trsf(trsf_lst **list, trsf_lst *new)
{
  if (*list)
    new->next = *list;
  *list = new;
}

/*
  Créé une nouvelle liste de transfert avec un seul transfert
  - fd_destinataire : le file descriptor du destinataire
*/
trsf_lst  *create_trsf(int fd_destinataire, int nb_msg, char *file_name)
{
  trsf_lst  *lst;
  static int  id = 0;

  if ((lst = (trsf_lst *)malloc(sizeof(trsf_lst))) == NULL)
    return NULL;

  if ((lst->file_name = strdup(file_name)) == NULL)
  {
    free(lst);
    return NULL;
  }
  lst->fd_destinataire = fd_destinataire;
  lst->id_transfert = id;
  lst->nb_msg = nb_msg;
  id++;
  lst->next = NULL;

  return lst;
}

/*
  Créé une nouvelle liste de transfert avec un seul transfert
  - fd_destinataire : le file descriptor du destinataire
*/
trsf_lst  *create_trsf2(int fd_destinataire, int nb_msg, char *file_name, int id_transfert)
{
  trsf_lst  *lst;

  if ((lst = (trsf_lst *)malloc(sizeof(trsf_lst))) == NULL)
    return NULL;

  if ((lst->file_name = strdup(file_name)) == NULL)
  {
    free(lst);
    return NULL;
  }
  lst->id_transfert = id_transfert;
  lst->fd_destinataire = fd_destinataire;
  lst->nb_msg = nb_msg;
  lst->next = NULL;

  return lst;
}

void print_trsf_list(trsf_lst **list)
{
  trsf_lst  *tmp;

  tmp = *list;
  while(tmp)
  {
    printf("id_transfert : %d\nfd_destinataire : %d\n", tmp->id_transfert, tmp->fd_destinataire);
    tmp = tmp->next;
  }
}

/*
  Supprime un transfert de la liste des transferts
  - list : la liste des transferts
  - id_transfert : l'id du transfert à supprimer
*/
void remove_trsf(trsf_lst **list, int id_transfert)
{
  trsf_lst  *current;
  trsf_lst  *previous;

  if (!*list)
    return ;
  previous = *list;
  current = previous->next;
  if (previous->id_transfert == id_transfert)
  {
    free(previous->file_name);
    free(previous);
    previous = NULL;
    *list = current;
  }
  else
  {
    while (current && current->id_transfert != id_transfert)
    {
      previous = current;
      current = current->next;
    }
    if (current)
    {
      previous->next = current->next;
      free(previous->file_name);
      free(current);
      current = NULL;
    }
  }
}

/*
  Supprime une une liste d'utilisateur
*/
void delete_list_trsf(trsf_lst **list)
{
  trsf_lst *tmp;
  while (*list != NULL)
  {
    tmp = (*list)->next;
    remove_trsf(list, (*list)->id_transfert);
    *list = tmp;
  }
}

trsf_lst  *find_trsf_id(trsf_lst **list, int id_transfert)
{
  trsf_lst  *tmp;

  tmp = *list;
  while(tmp)
  {
    if (tmp->id_transfert == id_transfert)
      return tmp;
    tmp = tmp->next;
  }
  return NULL;
}
