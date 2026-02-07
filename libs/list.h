#include "header.h"
#include <stdlib.h>

typedef struct list_element {
    Header header;
    struct list_element  *next;
} item;

typedef  item* list;

typedef  int boolean;

/* PRIMITIVE  */
list emptylist(void);
boolean empty(list);
list cons(Header, list);
Header head(list);
list tail(list);

void showlist(list);
void freelist(list);
int member(Header, list l);
list lastNode(list a);
int dimList(list a);
Header* listToArray(list a, int* dimLo);
list arrayToList(Header *v,int dim);

