#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    element_t *q = malloc(sizeof(element_t));
    if (q == NULL) {
        return NULL;
    }
    INIT_LIST_HEAD(&q->list);
    return &q->list;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (l == NULL) {
        return;
    }
    struct list_head *node = l->next;
    while (node != l) {
        // cppcheck-suppress nullPointer
        element_t *e = container_of(node, element_t, list);
        node = node->next;
        q_release_element(e);
    }
    // cppcheck-suppress nullPointer
    element_t *head = container_of(l, element_t, list);
    free(head);
    // bad
    /*
        if (!l)
            return;

        // iterate over the list entries and remove it
        element_t *entry, *safe;
        list_for_each_entry_safe (entry, safe, l, list)
            q_release_element(entry);
        free(l);
    */
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    /*
        if (!head)
            return false;
        element_t *ele = malloc(sizeof(element_t));
        if (!ele)
            return false;
        ele->value = malloc(sizeof(s));
        memset(ele->value, '\0', strlen(ele->value));
        strncpy(ele->value, s, strlen(s));
        list_add(&ele->list, head);
        return true;
    */
    if (!head)
        return false;

    // allocate memory for element_t
    element_t *new_entry = malloc(sizeof(element_t));
    if (!new_entry)
        return false;

    size_t len = strlen(s) + 1;
    // allocate memory for 'value' in element_t
    new_entry->value = malloc(len);
    if (!(new_entry->value)) {
        free(new_entry);
        return false;
    }
    memcpy(new_entry->value, s, len);

    list_add(&new_entry->list, head);

    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *ele = malloc(sizeof(element_t));
    if (!ele)
        return false;
    // causing corruption
    /*
    ele->value = malloc(sizeof(s));
    memset(ele->value, '\0', strlen(ele->value));
    strncpy(ele->value, s, strlen(s));
    */
    ele->value = strdup(s);
    list_add_tail(&ele->list, head);
    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    if (sp) {
        element_t *e = list_first_entry(head, element_t, list);
        size_t len = strlen(e->value);
        len = (bufsize - 1) > len ? len : (bufsize - 1);
        memcpy(sp, e->value, len);
        sp[len] = '\0';

        list_del(&e->list);
        return e;
    }
    return NULL;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    if (sp) {
        element_t *e = list_entry(head->prev, element_t, list);
        size_t len = strlen(e->value);
        len = (bufsize - 1) > len ? len : (bufsize - 1);
        memcpy(sp, e->value, len);
        sp[len] = '\0';

        list_del(&e->list);
        return e;
    }
    return NULL;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;
    int len = 0;
    struct list_head *li;
    list_for_each (li, head)
        len++;
    return len;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head)) {
        return false;
    }
    struct list_head *front = head->next;
    struct list_head *end = head->prev;
    while (true) {
        if (front == end || front->next == end) {
            list_del(front);
            q_release_element(list_entry(front, element_t, list));
            break;
        }
        front = front->next;
        end = end->prev;
    }
    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    printf("delete dup\n");
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head)
        return false;
    element_t *node, *tmp;
    char *prev_value = "";
    list_for_each_entry_safe (node, tmp, head, list) {
        if (strcmp(prev_value, node->value) == 0) {
            list_del(&node->list);
            free(node->value);
            free(node);
        } else {
            prev_value = node->value;
        }
    }
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head)
        return;
    struct list_head *node;
    for (node = head->next; (node->next != head) && (node != head);
         node = node->next) {
        struct list_head *next = node->next;
        list_del(node);
        list_add(node, next);
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *prev = head->prev, *curr = head, *next = NULL;

    while (next != head) {
        next = curr->next;
        curr->next = prev;
        curr->prev = next;
        prev = curr;
        curr = next;
    }
}
/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */

void q_sort(struct list_head *head) {}
