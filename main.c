#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMAND_ARGUMENTS                                                  \
  515 // 512 maximum number of cars + 1 for the vehicles counter + 1
      // for the station key + 1 for the command itself.

#define MAX_COMMAND_ARGUMENT_LENGTH                                            \
  19 // Each integer can be 10 digits long at most (INT_MAX = 2147483647); the
     // longest commands are "demolisci-stazione" and "pianifica-percorso" both
     // are 18 characters long. Including the null terminator, 19 characters are
     // enough.

// The maximum length of a command is 19 * 515 = 9785.
#define COMMAND_BUFFER_SIZE MAX_COMMAND_ARGUMENTS *MAX_COMMAND_ARGUMENT_LENGTH

// The delimiter between arguments is a space.
#define COMMAND_ARGUMENTS_DELIMITER " "

// The hash of each command is obtained by summing the ASCII values of each
// character (including "-").
#define aggiungi_stazione_HASH 1765
#define aggiungi_auto_HASH 1329
#define demolisci_stazione_HASH 1875
#define rottama_auto_HASH 1246
#define pianifica_percorso_HASH 1854

// Simplifies the roue planner implementation.
#define INFINITY 2147483647

/* CARS MANAGEMENT - BEGIN */

/* The following code implements a generic single-linked list to store the cars
 * ranges that are inside each station. To keep the route planner simpler the
 * structure keeps track of the highest key present in the list at every given
 * moment. */

typedef struct SLL_Node {
  int key;
  struct SLL_Node *next;
} SLL_Node;

typedef struct SLL_List {
  int maximumKey, nodesCount;
  struct SLL_Node *head;
} SLL_List;

// Returns a pointer to a newly created list.
SLL_List *SLL_Initialize() {
  SLL_List *L = (SLL_List *)malloc(sizeof(SLL_List));
  L->maximumKey = 0;
  L->nodesCount = 0;
  L->head = NULL;

  return L;
}

// Returns 0 if the insertion was successful, -1 otherwise.
int SLL_Insert(SLL_List *L, int key) {
  // Specification constraints check
  if (key < 0 || L->nodesCount == 512) {
    return -1;
  }
  // Perform insertion
  SLL_Node *newNode = (SLL_Node *)malloc(sizeof(SLL_Node));
  newNode->key = key;
  newNode->next = L->head;
  L->head = newNode;
  L->nodesCount++;
  // Update maximum key if needed
  if (key > L->maximumKey) {
    L->maximumKey = key;
  }

  return 0;
}

// Returns the maximum key present in the list.
int SLL_FindMaximum(SLL_List *L) {
  SLL_Node *currentNode = L->head;
  int maximum = 0;
  while (currentNode != NULL) {
    if (currentNode->key > maximum) {
      maximum = currentNode->key;
    }
    currentNode = currentNode->next;
  }

  return maximum;
}

// Returns 0 if the deletion was successful, -1 otherwise.
int SLL_Delete(SLL_List *L, int key) {
  // Specification constraints check
  if (key < 0 || L->nodesCount == 0) {
    return -1;
  }
  // Perform deletion
  SLL_Node *currentNode = L->head;
  SLL_Node *previousNode = NULL;
  while (currentNode != NULL) {
    if (currentNode->key == key) {
      if (previousNode == NULL) {
        L->head = currentNode->next;
      } else {
        previousNode->next = currentNode->next;
      }
      free(currentNode);
      L->nodesCount--;
      // Update maximum key if needed
      if (key == L->maximumKey) {
        L->maximumKey = SLL_FindMaximum(L);
      }

      return 0;
    }
    previousNode = currentNode;
    currentNode = currentNode->next;
  }

  return -1;
}

void SLL_Destroy(SLL_List *L) {
  SLL_Node *currentNode = L->head;
  SLL_Node *nextNode = NULL;
  while (currentNode != NULL) {
    nextNode = currentNode->next;
    free(currentNode);
    currentNode = nextNode;
  }
  free(L);
}

/* CARS MANAGEMENT - END */

/* STATIONS MANAGEMENT - BEGIN */

/* The following code implements a Red-black tree used to store each station of
   the highways as a node. Almost all the functions are implemented adhering to
   "Introduction to Algorithms, Fourth Edition" as close as possible; if a
   modification was necessary, it is specified in the comments (see "MOD"). All
   the functions expect the existence of a global pointer to the entry point of
   the tree "RBT_EntryPoint" which must be always accessible.
*/

typedef struct RBT_Node {
  int key;
  char color;
  struct RBT_Node *left, *right, *parent;
  // Extensions to the original structure.
  SLL_List *cars;
  int graphDepth;
  struct RBT_Node *previous;
} RBT_Node;

typedef struct RBT_Tree {
  struct RBT_Node *root;
  struct RBT_Node *nil;
} RBT_Tree;

RBT_Tree *RBT_EntryPoint;

void RBT_Initialize() {
  RBT_EntryPoint = (RBT_Tree *)malloc(sizeof(RBT_Tree));
  RBT_EntryPoint->nil = (RBT_Node *)malloc(sizeof(RBT_Node));
  RBT_EntryPoint->root = RBT_EntryPoint->nil;
  // The attributes of the nil node are arbitrary and set only for convenience.
  RBT_EntryPoint->nil->color = 'B';
  RBT_EntryPoint->nil->left = NULL;
  RBT_EntryPoint->nil->right = NULL;
  RBT_EntryPoint->nil->parent = NULL;
  RBT_EntryPoint->nil->key = -1;
}

void RBT_LeftRotate(RBT_Node *x) {
  RBT_Node *y = x->right;
  x->right = y->left;
  if (y->left != RBT_EntryPoint->nil) {
    y->left->parent = x;
  }
  y->parent = x->parent;
  if (x->parent == RBT_EntryPoint->nil) {
    RBT_EntryPoint->root = y;
  } else if (x == x->parent->left) {
    x->parent->left = y;
  } else {
    x->parent->right = y;
  }
  y->left = x;
  x->parent = y;
}

void RBT_RightRotate(RBT_Node *x) {
  RBT_Node *y = x->left;
  x->left = y->right;
  if (y->right != RBT_EntryPoint->nil) {
    y->right->parent = x;
  }
  y->parent = x->parent;
  if (x->parent == RBT_EntryPoint->nil) {
    RBT_EntryPoint->root = y;
  } else if (x == x->parent->right) {
    x->parent->right = y;
  } else {
    x->parent->left = y;
  }
  y->right = x;
  x->parent = y;
}

void RBT_InsertFixup(RBT_Node *z) {
  RBT_Node *y;
  while (z->parent->color == 'R') {
    if (z->parent == z->parent->parent->left) {
      y = z->parent->parent->right;
      if (y->color == 'R') {
        z->parent->color = 'B';
        y->color = 'B';
        z->parent->parent->color = 'R';
        z = z->parent->parent;
      } else {
        if (z == z->parent->right) {
          z = z->parent;
          RBT_LeftRotate(z);
        }
        z->parent->color = 'B';
        z->parent->parent->color = 'R';
        RBT_RightRotate(z->parent->parent);
      }
    } else {
      y = z->parent->parent->left;
      if (y->color == 'R') {
        z->parent->color = 'B';
        y->color = 'B';
        z->parent->parent->color = 'R';
        z = z->parent->parent;
      } else {
        if (z == z->parent->left) {
          z = z->parent;
          RBT_RightRotate(z);
        }
        z->parent->color = 'B';
        z->parent->parent->color = 'R';
        RBT_LeftRotate(z->parent->parent);
      }
    }
  }
  RBT_EntryPoint->root->color = 'B';
}

// Returns a pointer to the newly inserted node or to the nil node if the key is
// already present or invalid.
RBT_Node *RBT_Insert(int key) {
  // MOD: The key must be a positive integer.
  if (key < 0) {
    return RBT_EntryPoint->nil;
  }
  RBT_Node *y = RBT_EntryPoint->nil;
  RBT_Node *x = RBT_EntryPoint->root;
  while (x != RBT_EntryPoint->nil) {
    // MOD: The key must be unique.
    y = x;
    if (key < x->key) {
      x = x->left;
    } else if (key > x->key) {
      x = x->right;
    } else {
      return RBT_EntryPoint->nil;
    }
  }
  RBT_Node *z = (RBT_Node *)malloc(sizeof(RBT_Node));
  z->key = key;
  z->parent = y;
  if (y == RBT_EntryPoint->nil) {
    RBT_EntryPoint->root = z;
  } else if (z->key < y->key) {
    y->left = z;
  } else {
    y->right = z;
  }
  z->left = RBT_EntryPoint->nil;
  z->right = RBT_EntryPoint->nil;
  z->color = 'R';
  RBT_InsertFixup(z);

  return z;
}

void RBT_Transplant(RBT_Node *u, RBT_Node *v) {
  if (u->parent == RBT_EntryPoint->nil) {
    RBT_EntryPoint->root = v;
  } else if (u == u->parent->left) {
    u->parent->left = v;
  } else {
    u->parent->right = v;
  }
  v->parent = u->parent;
}

// Returns a pointer to the node with the minimum key in the subtree rooted at
// x.
RBT_Node *RBT_Minimum(RBT_Node *x) {
  while (x->left != RBT_EntryPoint->nil) {
    x = x->left;
  }

  return x;
}

void RBT_DeleteFixup(RBT_Node *x) {
  RBT_Node *w;
  while (x != RBT_EntryPoint->root && x->color == 'B') {
    if (x == x->parent->left) {
      w = x->parent->right;
      if (w->color == 'R') {
        w->color = 'B';
        x->parent->color = 'R';
        RBT_LeftRotate(x->parent);
        w = x->parent->right;
      }
      if (w->left->color == 'B' && w->right->color == 'B') {
        w->color = 'R';
        x = x->parent;
      } else {
        if (w->right->color == 'B') {
          w->left->color = 'B';
          w->color = 'R';
          RBT_RightRotate(w);
          w = x->parent->right;
        }
        w->color = x->parent->color;
        x->parent->color = 'B';
        w->right->color = 'B';
        RBT_LeftRotate(x->parent);
        x = RBT_EntryPoint->root;
      }
    } else {
      w = x->parent->left;
      if (w->color == 'R') {
        w->color = 'B';
        x->parent->color = 'R';
        RBT_RightRotate(x->parent);
        w = x->parent->left;
      }
      if (w->right->color == 'B' && w->left->color == 'B') {
        w->color = 'R';
        x = x->parent;
      } else {
        if (w->left->color == 'B') {
          w->right->color = 'B';
          w->color = 'R';
          RBT_LeftRotate(w);
          w = x->parent->left;
        }
        w->color = x->parent->color;
        x->parent->color = 'B';
        w->left->color = 'B';
        RBT_RightRotate(x->parent);
        x = RBT_EntryPoint->root;
      }
    }
  }
  x->color = 'B';
}

// RBT_Delete just "unlinks" the given node from the tree, it DOESN'T free the
// memory! This is done by the caller so that nested data structures can be
// freed in the correct way and order.
void RBT_Delete(RBT_Node *z) {
  RBT_Node *y = z;
  RBT_Node *x;
  char yOriginalColor = y->color;
  if (z->left == RBT_EntryPoint->nil) {
    x = z->right;
    RBT_Transplant(z, z->right);
  } else if (z->right == RBT_EntryPoint->nil) {
    x = z->left;
    RBT_Transplant(z, z->left);
  } else {
    y = RBT_Minimum(z->right);
    yOriginalColor = y->color;
    x = y->right;
    if (y != z->right) {
      RBT_Transplant(y, y->right);
      y->right = z->right;
      y->right->parent = y;
    } else {
      x->parent = y;
    }
    RBT_Transplant(z, y);
    y->left = z->left;
    y->left->parent = y;
    y->color = z->color;
  }
  if (yOriginalColor == 'B') {
    RBT_DeleteFixup(x);
  }
}

// Returns a pointer to the node with the given key or to the nil node if the
// key is not present.
RBT_Node *RBT_Search(int key) {
  RBT_Node *x = RBT_EntryPoint->root;
  while (x != RBT_EntryPoint->nil && key != x->key) {
    if (key < x->key) {
      x = x->left;
    } else {
      x = x->right;
    }
  }

  return x;
}

// Returns a pointer to the successor of the given node or to the nil node if it
// doesn't exist.
RBT_Node *RBT_Successor(RBT_Node *x) {
  if (x->right != RBT_EntryPoint->nil) {
    return RBT_Minimum(x->right);
  }
  RBT_Node *y = x->parent;
  while (y != RBT_EntryPoint->nil && x == y->right) {
    x = y;
    y = y->parent;
  }

  return y;
}

// Returns a pointer to the node with the maximum key in the subtree rooted at
// x.
RBT_Node *RBT_Maximum(RBT_Node *x) {
  while (x->right != RBT_EntryPoint->nil) {
    x = x->right;
  }

  return x;
}

// Returns a pointer to the predecessor of the given node or to the nil node if
// it doesn't exist.
RBT_Node *RBT_Predecessor(RBT_Node *x) {
  if (x->left != RBT_EntryPoint->nil) {
    return RBT_Maximum(x->left);
  }
  RBT_Node *y = x->parent;
  while (y != RBT_EntryPoint->nil && x == y->left) {
    x = y;
    y = y->parent;
  }

  return y;
}

/* STATIONS MANAGEMENT - END */

/* ROUTE PLANNER - BEGIN */

void PF_ResetNodes(RBT_Node *startStation, RBT_Node *endStation) {
  RBT_Node *currentNode = startStation;
  while (currentNode != endStation) {
    currentNode->graphDepth = INFINITY;
    currentNode->previous = NULL;
    currentNode = RBT_Successor(currentNode);
  }
  endStation->graphDepth = INFINITY;
  endStation->previous = NULL;
}

// Returns 0 if the route was found, 1 otherwise. Use this function only if the
// startStation is before the endStation.
int PF_NS_PlanRoute(RBT_Node *startStation, RBT_Node *endStation) {
  RBT_Node *currentStation = startStation;
  currentStation->graphDepth = 0;
  int discovered = currentStation->key;
  while (currentStation->key <= endStation->key &&
         currentStation != RBT_EntryPoint->nil) {
    if (currentStation->key > discovered) {
      return 1;
    }
    if (currentStation == endStation) {
      return 0;
    }
    if ((currentStation->key + currentStation->cars->maximumKey) > discovered) {
      RBT_Node *tmp = RBT_Successor(currentStation);
      while (tmp->key <=
                 (currentStation->key + currentStation->cars->maximumKey) &&
             tmp->key <= endStation->key && tmp != RBT_EntryPoint->nil) {
        if (tmp->graphDepth > currentStation->graphDepth + 1) {
          tmp->graphDepth = currentStation->graphDepth + 1;
          tmp->previous = currentStation;
        } else if (tmp->graphDepth == currentStation->graphDepth + 1) {
          if (tmp->previous->key > currentStation->key) {
            tmp->previous = currentStation;
          }
        }
        tmp = RBT_Successor(tmp);
      }
      discovered = currentStation->key + currentStation->cars->maximumKey;
    }
    currentStation = RBT_Successor(currentStation);
  }
  return 1;
}

// Returns 0 if the route was found, 1 otherwise. Use this function only if the
// startStation is after the endStation.
int PF_SN_PlanRoute(RBT_Node *startStation, RBT_Node *endStation) {
  RBT_Node *currentStation = startStation;
  currentStation->graphDepth = 0;
  int discovered = currentStation->key;
  while (currentStation->key >= endStation->key &&
         currentStation != RBT_EntryPoint->nil) {
    if (currentStation->key < discovered) {
      return 1;
    }
    if (currentStation == endStation) {
      return 0;
    }
    RBT_Node *tmp = RBT_Predecessor(currentStation);
    while (tmp->key >=
               (currentStation->key - currentStation->cars->maximumKey) &&
           tmp->key >= endStation->key && tmp != RBT_EntryPoint->nil) {
      if (tmp->graphDepth > currentStation->graphDepth + 1) {
        tmp->graphDepth = currentStation->graphDepth + 1;
        tmp->previous = currentStation;
      } else if (tmp->graphDepth == currentStation->graphDepth + 1) {
        if (tmp->previous->key > currentStation->key) {
          tmp->previous = currentStation;
        }
      }
      tmp = RBT_Predecessor(tmp);
    }
    if (currentStation->key - currentStation->cars->maximumKey < discovered) {
      discovered = currentStation->key - currentStation->cars->maximumKey;
    }
    currentStation = RBT_Predecessor(currentStation);
  }
  return 1;
}

void PF_PrintRoute(RBT_Node *station) {
  if (station->previous == NULL) {
    printf("%d", station->key);
    return;
  }
  PF_PrintRoute(station->previous);
  printf(" %d", station->key);
}

/* ROUTE PLANNER - END */

/* PROJECT RELATED FUNCTIONS - BEGIN */

void PRJ_AggiungiStazione(int x, int y, int *vehicles) {
  RBT_Node *newStation = RBT_Insert(x);
  if (newStation == RBT_EntryPoint->nil) {
    printf("non aggiunta\n");
    return;
  }
  newStation->cars = SLL_Initialize();
  newStation->graphDepth = INFINITY;
  newStation->previous = NULL;
  int i, k;
  for (i = 0; i < y; i++) {
    k = SLL_Insert(newStation->cars, vehicles[i]);
    if (k == -1) {
      printf("CRITICAL ERROR: SLL_Insert failed!\n");
      return;
    }
  }
  printf("aggiunta\n");
  return;
}

void PRJ_AggiungiAuto(int x, int y) {
  RBT_Node *station = RBT_Search(x);
  if (station == RBT_EntryPoint->nil) {
    printf("non aggiunta\n");
    return;
  }
  int k = SLL_Insert(station->cars, y);
  if (k == -1) {
    printf("CRITICAL ERROR: SLL_Insert failed!\n");
    return;
  }
  printf("aggiunta\n");
  return;
}

void PRJ_DemolisciStazione(int x) {
  RBT_Node *station = RBT_Search(x);
  if (station == RBT_EntryPoint->nil) {
    printf("non demolita\n");
    return;
  }
  RBT_Delete(station);
  SLL_Destroy(station->cars);
  free(station);
  printf("demolita\n");
  return;
}

void PRJ_RottamaAuto(int x, int y) {
  RBT_Node *station = RBT_Search(x);
  if (station == RBT_EntryPoint->nil) {
    printf("non rottamata\n");
    return;
  }
  int k = SLL_Delete(station->cars, y);
  if (k == -1) {
    printf("non rottamata\n");
    return;
  }
  printf("rottamata\n");
  return;
}

void PRJ_PianificaPercorso(int x, int y) {
  RBT_Node *startStation = RBT_Search(x);
  RBT_Node *endStation = RBT_Search(y);
  // Case 0: one or both of the stations don't exist.
  if (startStation == RBT_EntryPoint->nil ||
      endStation == RBT_EntryPoint->nil) {
    printf("nessun percorso\n");
    return;
  }
  // Case 1: the stations are the same.
  if (startStation == endStation) {
    printf("%d\n", startStation->key);
    return;
  }
  // Case 2: we need to take the highway from north to south.
  if (endStation->key > startStation->key) {
    PF_ResetNodes(startStation, endStation);
    if (PF_NS_PlanRoute(startStation, endStation) == 1) {
      printf("nessun percorso\n");
      return;
    }
    PF_PrintRoute(endStation);
    printf("\n");
    return;
  }
  // Case 3: we need to take the highway from south to north.
  if (endStation->key < startStation->key) {
    PF_ResetNodes(endStation, startStation);
    if (PF_SN_PlanRoute(startStation, endStation) == 1) {
      printf("nessun percorso\n");
      return;
    }
    PF_PrintRoute(endStation);
    printf("\n");
    return;
  }
  return;
}

/* PROJECT RELATED FUNCTIONS - END */

/* UTILITY FUNCTIONS - BEGIN*/

// Returns the hash of the given string.
int UTILS_hashString(char *command) {
  int hash = 0;
  for (int i = 0; i < strlen(command); i++) {
    hash += command[i];
  }
  return hash;
}

void UTILS_CommandsHandler() {
  char commandBuffer[COMMAND_BUFFER_SIZE];
  char *commandArgumentHolder;
  int x, y, i, tmp;
  int *vehicles;

  while (fgets(commandBuffer, COMMAND_BUFFER_SIZE, stdin)) {
    commandArgumentHolder = strtok(commandBuffer, COMMAND_ARGUMENTS_DELIMITER);
    tmp = UTILS_hashString(commandArgumentHolder);
    switch (tmp) {
    case aggiungi_stazione_HASH:
      commandArgumentHolder = strtok(NULL, COMMAND_ARGUMENTS_DELIMITER);
      x = atoi(commandArgumentHolder);
      commandArgumentHolder = strtok(NULL, COMMAND_ARGUMENTS_DELIMITER);
      y = atoi(commandArgumentHolder);
      vehicles = (int *)malloc(sizeof(int) * MAX_COMMAND_ARGUMENTS);
      for (i = 0; i < y; i++) {
        commandArgumentHolder = strtok(NULL, COMMAND_ARGUMENTS_DELIMITER);
        vehicles[i] = atoi(commandArgumentHolder);
      }
      PRJ_AggiungiStazione(x, y, vehicles);
      free(vehicles);
      break;
    case aggiungi_auto_HASH:
      commandArgumentHolder = strtok(NULL, COMMAND_ARGUMENTS_DELIMITER);
      x = atoi(commandArgumentHolder);
      commandArgumentHolder = strtok(NULL, COMMAND_ARGUMENTS_DELIMITER);
      y = atoi(commandArgumentHolder);
      PRJ_AggiungiAuto(x, y);
      break;
    case demolisci_stazione_HASH:
      commandArgumentHolder = strtok(NULL, COMMAND_ARGUMENTS_DELIMITER);
      x = atoi(commandArgumentHolder);
      PRJ_DemolisciStazione(x);
      break;
    case rottama_auto_HASH:
      commandArgumentHolder = strtok(NULL, COMMAND_ARGUMENTS_DELIMITER);
      x = atoi(commandArgumentHolder);
      commandArgumentHolder = strtok(NULL, COMMAND_ARGUMENTS_DELIMITER);
      y = atoi(commandArgumentHolder);
      PRJ_RottamaAuto(x, y);
      break;
    case pianifica_percorso_HASH:
      commandArgumentHolder = strtok(NULL, COMMAND_ARGUMENTS_DELIMITER);
      x = atoi(commandArgumentHolder);
      commandArgumentHolder = strtok(NULL, COMMAND_ARGUMENTS_DELIMITER);
      y = atoi(commandArgumentHolder);
      PRJ_PianificaPercorso(x, y);
      break;
    default:
      return;
      break;
    }
  }
}

/* UTILITY FUNCTIONS - END*/

int main() {
  RBT_Initialize();
  UTILS_CommandsHandler();
  return 0;
}
