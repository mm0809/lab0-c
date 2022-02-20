#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h>

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
    struct list_head *queue = malloc(sizeof(struct list_head));
    if (queue != NULL)
        INIT_LIST_HEAD(queue);

    return queue;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;

    // iterate over the list entries and remove it
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, l, list)
        q_release_element(entry);
    free(l);
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

    list_add_tail(&new_entry->list, head);

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

    element_t *entry = list_first_entry(head, element_t, list);
    list_del(&entry->list);

    if (sp) {
        size_t len = strlen(entry->value);
        len = (bufsize - 1) > len ? len : (bufsize - 1);
        memcpy(sp, entry->value, len);
        sp[len] = '\0';
    }

    return entry;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *entry = list_last_entry(head, element_t, list);
    list_del(&entry->list);

    if (sp) {
        size_t len = strlen(entry->value);
        len = (bufsize - 1) > len ? len : (bufsize - 1);
        memcpy(sp, entry->value, len);
        sp[len] = '\0';
    }

    return entry;
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
    struct list_head *trv;

    list_for_each (trv, head)
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
    if (!head || list_empty(head))
        return false;

    struct list_head *slow, *fast;
    for (slow = head->next, fast = slow->next;
         fast != head && fast != head->prev;
         slow = slow->next, fast = fast->next->next)
        ;

    slow = (fast == head) ? slow : slow->next;
    list_del(slow);
    q_release_element(list_entry(slow, element_t, list));

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
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head)
        return false;
    if (list_empty(head))
        return true;

    const char *last_value = NULL;
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list) {
        if (!last_value)
            last_value = entry->value;
        else {
            if (strcmp(last_value, entry->value) == 0) {
                list_del(&entry->list);
                q_release_element(entry);
            } else {
                last_value = entry->value;
            }
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
    if (!head || list_is_singular(head))
        return;

    struct list_head *n1, *n2;
    for (n1 = head->next, n2 = n1->next; n1 != head && n2 != head;
         n1 = n1->next, n2 = n1->next) {
        list_del(n2);
        n2->prev = n1->prev;
        n2->next = n1;
        n1->prev->next = n2;
        n1->prev = n2;
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
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    const struct list_head *const first = head->next;
    for (struct list_head *new_first = first->next; first->next != head;
         new_first = first->next) {
        list_del(new_first);
        list_add(new_first, head);
    }
}

struct list_head *mergesort(struct list_head *head);
struct list_head *merge(struct list_head *l1, struct list_head *l2);
/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    // break the structure of circular linked list
    head->prev->next = NULL;

    head->next = mergesort(head->next);

    struct list_head *prev = head, *cur = head->next;
    while (cur->next != NULL) {
        cur->prev = prev;
        prev = cur;
        cur = cur->next;
    }
    cur->prev = prev;
    cur->next = head;
    head->prev = cur;
}

struct list_head *mergesort(struct list_head *head)
{
    if (!head || !head->next)
        return head;

    struct list_head *slow = head, *fast = head->next;
    while (fast && fast->next) {
        fast = fast->next->next;
        slow = slow->next;
    }
    fast = slow->next;
    slow->next = NULL;
    slow = head;

    struct list_head *l1 = mergesort(slow);
    struct list_head *l2 = mergesort(fast);

    return merge(l1, l2);
}

struct list_head *merge(struct list_head *l1, struct list_head *l2)
{
    struct list_head *nl = NULL, **ptr = &nl;

    while (l1 && l2) {
        if (strcmp(list_entry(l1, element_t, list)->value,
                   list_entry(l2, element_t, list)->value) < 0) {
            *ptr = l1;
            l1 = l1->next;
        } else {
            *ptr = l2;
            l2 = l2->next;
        }
        ptr = &(*ptr)->next;
    }
    *ptr = (struct list_head *) ((uintptr_t) l1 | (uintptr_t) l2);
    return nl;
}
