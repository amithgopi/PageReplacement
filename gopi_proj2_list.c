/**
 * @file list.c
 * @author Amith Gopi (amithgopi@tamu.edu)
 * @brief Implement a simple doubly linked list for integer type values
 * @version 1.0
 * @date 2022-03-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Acts as a node in the struct. Holds the value and the next and previous pointers
 * 
 */
struct Node{
    int pageFrame;
    struct Node *next;
    struct Node *previous;
} Node;

/**
 * @brief Wrapper struct for the list. Contains the front and back pointers and size of list.
 * 
 */
struct List{
    struct Node *front;
    struct Node *back;
    int size;
} ;

/**
 * @brief Add a entry to the front of the queue
 * 
 * @param list Pointer to the list
 * @param page Page number
 */
void push_front(struct List* list, int page) {
    struct Node *n = malloc(sizeof(struct Node));
    n->pageFrame = page; n->next= list->front; n->previous = NULL;
    if (list->front == NULL)  list->back = n;   
    else list->front->previous = n;     
    list->front = n;
    list->size++;
}

/**
 * @brief Pop an entry from the back of the list
 * 
 * @param list Pointer to the list
 * @return int Value of the element removed, if null return -1
 */
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

/**
 * @brief Find the pointer to the given page number in the list
 * 
 * @param list Pointer to the list
 * @param page page number to search for
 * @return struct Node* Poiner to first element in the list where the page is located, NULL is not found
 */
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

/**
 * @brief Remove the node given by its pointer from the list
 * 
 * @param list Pointer to the list
 * @param n Pointer to the node
 * @return int value of the page beind removed, else return -1
 */
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

/**
 * @brief Find a given page in the list and rmeove its corresponding entry
 * 
 * @param list Pointer to the list
 * @param page page number to search for
 * @return int value of the page beind removed, else return -1
 */
int remove_page(struct List* list, int page) {
    struct Node* n = find(list, page);
    int val;
    val = remove_element(list, n);
}

/**
 * @brief Get the value of entry at back of the list
 * 
 * @param list Pointer to the list
 * @return int value of entry at back of the list
 */
int get_back(struct List* list) {
    return list->back->pageFrame;
}

/**
 * @brief Get the value of entry at front of the list
 * 
 * @param list Pointer to the list
 * @return int value of entry at back of the list
 */
int get_front(struct List *list){
    return list->front->pageFrame;
}

/**
 * @brief Print out the elements of the given list in the a formatted manner
 * 
 * @param list 
 */
void print_list(struct List* list) {
    struct Node* curr = list->front;
    while(curr != NULL) {
         printf("%d -> ", curr->pageFrame);
         curr = curr->next;
    }
    printf("\n");
   
}