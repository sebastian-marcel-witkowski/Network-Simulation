#include <iostream>

#include "msg_list.h"

msg_list::msg_list() {
	front = back = NULL;
}

void msg_list::display() {
	msg_list_node *tmp;  int i;
	
	if(front==NULL) {
		cout << "** List is empty. **\n";
		return;
	}
	
	tmp = front;  i = 1;
	while(tmp!=NULL) {
		cout << "Datagram " << i++ << ":  \n";
		(tmp->d)->display();
		cout << "\n";
		tmp = tmp->next;
	}
	
}

datagram* msg_list::returnFront() {
    if (front == NULL) return NULL;
    msg_list_node* tmp = front;
    datagram* x = tmp->d;
    front = front->next;
    if (front == NULL) back= NULL;
    delete tmp;
    return x;
}

void msg_list::append(datagram *x) {
	msg_list_node* tmp = new msg_list_node;
	tmp->next = NULL;
	tmp->d = x;
    
	if(front==NULL)
        	front = tmp;
	else
        	back->next = tmp;
		back = tmp;
}

/**************new**************/
void msg_list::deleteList() {
    while (front != NULL) {
        msg_list_node* tmp = front;
        front = front->next;
        delete tmp->d;
        delete tmp;
    }
    back = NULL;
}
/**************new**************/
