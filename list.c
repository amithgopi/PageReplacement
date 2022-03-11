#include <stdio.h>
#include <stdlib.h>

struct Node{
    int pageFrame;
    struct Node *next;
    struct Node *previous;
} Node;

struct List{
    struct Node *front;
    struct Node *back;
    int size;
} ;

void push_front(struct List* list, int page) {
    struct Node *n = malloc(sizeof(struct Node));
    n->pageFrame = page; n->next= list->front; n->previous = NULL;
    if (list->front == NULL)  list->back = n;   
    else list->front->previous = n;     
    list->front = n;
    list->size++;
}

int pop_back(struct List* list) {
    if (list->back == NULL) {
        return -1;
    } else if(list->front == list->back) {
        int val = list->back->pageFrame;
        free(list->back);
        list->front = NULL;
        list->back = NULL;
        list->size--;
        return val;
    } else {
        int val = list->back->pageFrame;
        free(list->back);
        list->back = list->back->previous; 
        if (list->back == NULL) list->front = NULL;
        else list->back->next = NULL;
        list->size--;
        return val;
    }
}

struct Node* find(struct List *list, int page) {
    struct Node* curr = list->front;
    int location = 0;
    while(curr != NULL) {
        if(curr->pageFrame == page) {
            return curr;
        }
        curr = curr->next;
    }
}

int remove_page(struct List* list, int page) {
    struct Node* n = find(list, page);
    int val;
    val = remove_element(list, n);
}

int remove_element(struct List* list, struct Node* n) {
    if(n == NULL) {
        return -1;
    } else {
        int val = n->pageFrame;

        if(n == list->front) { list->front = n->next; } 
        else { n->previous->next = n->next; }

        if(n == list->back) { list->back = n->previous;} 
        else { n->next->previous = n->previous;}

        free(n);
        list->size--;
        return val;
    }
}

int get_back(struct List* list) {
    return list->back->pageFrame;
}

int get_front(struct List *list){
    return list->front->pageFrame;
}

void print_list(struct List* list) {
    struct Node* curr = list->front;
    while(curr != NULL) {
         printf("%d -> ", curr->pageFrame);
         curr = curr->next;
    }
    printf("\n");
   
}