#include "tchatche.h"

/*
  Donne la longueur de la liste passée en argument
*/
int lst_length(user_lst **list)
{
  user_lst  *tmp;
  int       n;

  n = 0;
  tmp = *list;
  while(tmp)
  {
    n++;
    tmp = tmp->next;
  }

  return n;
}

/*
  Cette fonction permet d'ajouter un utilisateur a une liste d'utilisateurs
  - list : la liste des utilisateurs
  - new : le nouvel utilisateur
*/
void add_user (user_lst **list, user_lst *new)
{
  if (*list)
    new->next = *list;
  *list = new;
}

/*
  Cette fonction créé une nouvelle liste d'utilisateur avec un seul utilisateur
  Cette liste sera ajoutée à la liste des utilisateurs ensuite
  - pseudo : le pseudo de l'utilisateur
  - tube : le nom du tube de l'utilisateur
*/
user_lst  *create_user(char *pseudo, int fd)
{
  user_lst    *lst;
  static int  id = 0;

  if ((lst = (user_lst *)malloc(sizeof(user_lst))) == NULL)
    return NULL;
  //On initialise le pseudo en copiant l'argument pseudo (et non en le mettant directement)
  if ((lst->name = strdup(pseudo)) == NULL)
  {
    free(lst);
    return NULL;
  }

  lst->tube = fd;
  lst->id = id;
  id++;
  lst->next = NULL;

  return lst;
}

void print_user_list(user_lst **list)
{
  user_lst  *tmp;

  tmp = *list;
  while(tmp)
  {
    printf("Pseudo : %s\nId : %d\n\n", tmp->name, tmp->id);
    printf("fd : %d\n", tmp->tube);
    tmp = tmp->next;
  }
}

/*
  Cette fonction permet de supprimer un utilisateur d'une liste d'utilisateurs
  - list : la liste de utilisateurs
  - id : l'id de l'utilisateur à supprimer
*/
void remove_user (user_lst **list, int id)
{
  user_lst  *current;
  user_lst  *previous;

  if (!*list)
    return ;
  previous = *list;
  current = previous->next;
  if (previous->id == id)
  {
    free(previous->name);
    free(previous);
    previous = NULL;
    *list = current;
  }
  else
  {
    while (current && current->id != id)
    {
      previous = current;
      current = current->next;
    }
    if (current)
    {
      previous->next = current->next;
      free(current->name);
      free(current);
      current = NULL;
    }
  }
}

/*
  Supprime une une liste d'utilisateur
*/
void delete_list_user(user_lst **list)
{
  user_lst *tmp;
  while (*list != NULL)
  {
    tmp = (*list)->next;
    remove_user(list, (*list)->id);
    *list = tmp;
  }
}

user_lst  *find_user_id(user_lst **list, int id)
{
  user_lst  *tmp;

  tmp = *list;
  while(tmp)
  {
    if (tmp->id == id)
      return tmp;
    tmp = tmp->next;
  }
  return NULL;
}

user_lst  *find_user_pseudo(user_lst **list, char *pseudo)
{
  user_lst   *tmp;

  tmp = *list;
  while(tmp)
  {
    if(strcmp(tmp->name, pseudo) == 0)
      return tmp;
    tmp = tmp->next;
  }
  return NULL;
}
