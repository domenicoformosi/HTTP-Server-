#include "list.h"

/* OPERAZIONI PRIMITIVE */
list  emptylist(void)        /* costruttore lista vuota */
{
    return NULL;
}

boolean  empty(list l)    /* verifica se lista vuota */
{
    return (l == NULL);
}

list  cons(Header e, list l)
{
    list t;       /* costruttore che aggiunge in testa alla lista */
    t = (list)malloc(sizeof(item));
    t->value = e;
    t->next = l;
    return(t);
}

Header head(list l) /* selettore testa lista */
{
    if (empty(l)) exit(-2);
    else return (l->value);
}

list  tail(list l)         /* selettore coda lista */
{
    if (empty(l)) exit(-1);
    else return (l->next);
}

void showlist(list l) {
    Header temp;
    if (!empty(l)) {
        temp = head(l);
        printf("%d\n",temp);
        showlist(tail(l));
        return;
    }
    else {
        printf("\n\n");
        return;
    }
}


int member(Header el, list l) {
    int result = 0;
    while (!empty(l) && !result) {
        result = (el == head(l));
        if (!result)
            l = tail(l);
    }
    return result;
}


void freelist(list l) {
    if (empty(l))
        return;
    else {
        freelist(tail(l));
        free(l);
    }
    return;
}
Header* listToArray(list a, int* dimLo){
    int dim = dimList(a),i=0,j;
    Header * pointer = (Header*)malloc(dim*sizeof(Header)), *vet = (Header*)malloc(dim*sizeof(Header)),*temp;
    temp = pointer;
    while(!empty(a))
    {
        *temp = head(a);
        a = tail(a);
        temp++;
        
    }
    for (i=dim-1,j=0;i>=0;j++,i--)
    {
        vet[j] = pointer[i];
    }
    free(pointer);
    *dimLo = dim;
    return vet;
}

list arrayToList(Header *v,int dim)
{
    list l=emptylist();
    for (int i=0;i<dim;i++)
    {
        l = cons(v[i],l);
    }
    free(v);
    return l;
}

list lastNode(list a)
{
    list lastNode = emptylist();
    int flag = 0;
    if (empty(a))
    {
        exit(-1);
    }
    while (!empty(a))
    {
        if (a->next == NULL)
        {
            lastNode = a;
            flag=1;
        }
        a = tail(a);
    }
    return lastNode;
}
int dimList(list a)
{
    int lenght=0;
    if (empty(a))
    {
        return 0;
    }
    while (!empty(a))
    {
        lenght++;
        a = tail(a);
    }
    return lenght;
}
void sortList(list *a)
{
    //scegliere algoritmo che vuoi
    int *v = NULL,dim;
    v= listToArray(*a, &dim);
    quickSort(v, dim);
    *a = arrayToList(v, dim);
    
}
 

