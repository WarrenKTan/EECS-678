#ifndef LIST_QUESTIONS_H
#define LIST_QUESTIONS_H

#include "LinkedList.h"
#include <stdbool.h>
/*
 * Problem: Determine if a linked list has a cycle
 *
 * Description:
 * Given the head of a linked list, determine if the list contains a cycle. A cycle occurs if a node can be
 * reached again by continuously following the `next` pointer. Internally, 'pos' is used to denote the index
 * of the node that the tail's next pointer is connected to, indicating a cycle. Note that 'pos' is not
 * accessible or passed as a parameter; it's only used for problem understanding and explanation.
 *
 * Task:
 * Implement a function to check if the given linked list has a cycle. The function should return 'true' if a
 * cycle is present and 'false' otherwise.
 *
 * Prototype:
 * bool hasCycle(struct Node *head);
 */

static bool hasCycle(struct Node *head)
{
	// initialize tracking pointers
	struct Node* slow = head;
	struct Node* fast = head;

	// make sure fast is not at end of list before incrementing AND that it won't run into a NULL pointer
	while(fast != NULL && fast->next != NULL){
		// increment slow pointer every cycle
		slow = slow->next;

		// increment fast pointer twice every cycle
		fast = fast->next->next;

		// check for cyclical linked list
		if(slow == fast){
            return true;
        }
	}

	// fast has reached the end of the list
	return false;
}

/*
 * Problem: Merge Two Sorted Lists
 *
 * Description:
 * You are given the heads of two sorted linked lists, list1 and list2. Your task is to merge these two
 * lists into one single sorted list. The merged list should be constructed by splicing together the nodes
 * of the first two lists without creating new nodes, but by rearranging the nodes from the given lists.
 *
 * Task:
 * Implement a function that merges two sorted linked lists and returns the head of the newly merged sorted
 * linked list.
 *
 * Prototype:
 * struct Node* mergeTwoLists(struct Node* list1, struct Node* list2);
 *
 * Note:
 * Both list1 and list2 are sorted in non-decreasing order.
 */

static struct Node* mergeLists(struct Node* list1, struct Node* list2)
{
	// initialise list that gets deleted upon return
	struct Node merged;

	// tail starts at the end of the merged list
    struct Node* tail = &merged;

	// continuously add the smaller int to the merged list's tail only one of the original lists are left
    while (list1 != NULL && list2 != NULL) {
		// find smaller int of the two heads
        if(list1->data <= list2->data){
			// add head to tail of merged list
            tail->next = list1;
            
			// shift head pointer
			list1 = list1->next;
        }else{
			// add head to tail of merged list
            tail->next = list2;

			// shift head pointer
            list2 = list2->next;
        }

		// shift tail to end of list
        tail = tail->next;
    }

	// add remaining list to the end of merged list
	if(list1 == NULL){
		tail->next = list2;
	}else{
		tail->next = list1;
	}
	
	//Placeholder return statement
	return merged.next;
}

#endif
