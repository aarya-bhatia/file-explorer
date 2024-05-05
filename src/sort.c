#include "sort.h"

/* sorts the linked list by changing next pointers (not data) */
File *merge_sort(File *head, file_compare cmp)
{
    File *a = NULL;
    File *b = NULL;

    /* Base case -- length 0 or 1 */
    if (!head || !head->next) {
        return head;
    }

    /* Split head into 'a' and 'b' sublists */
    split_list(head, &a, &b);

    /* Recursively sort the sublists */
    a = merge_sort(a, cmp);
    b = merge_sort(b, cmp);

    return merge_sorted(a, b, cmp);
}

/* combines sorted lists using comparator and returns new head pointer */
File *merge_sorted(File *a, File *b, file_compare cmp)
{
    File *result = NULL;

    /* Base cases */
    if (!a) {
        return b;
    }

    if (!b) {
        return a;
    }

    /* Pick either a or b, and recur */
    if (cmp(a, b) < 0) {
        result = a;
        result->next = merge_sorted(a->next, b, cmp);
    } else {
        result = b;
        result->next = merge_sorted(a, b->next, cmp);
    }

    return result;
}

/* Split the nodes of the given list into front and back halves . */
void split_list(File *source, File **front_ptr, File **back_ptr)
{
    File *fast;
    File *slow;
    slow = source;
    fast = source->next;

    /* Advance 'fast' two nodes, and advance 'slow' one node */
    while (fast != NULL) {
        fast = fast->next;
        if (fast != NULL) {
            slow = slow->next;
            fast = fast->next;
        }
    }

    /* 'slow' is before the midpoint in the list, so split it in two at that point. */
    *front_ptr = source;
    *back_ptr = slow->next;
    slow->next = NULL;
}
