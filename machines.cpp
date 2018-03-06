#include <iostream>
#include <stdlib.h>
#include <string>

#include "machines.h"

//*****************************************//
// node class functions

node::node(string n, IPAddress a) {
    name = new string;
    *name = n;
    my_address = a;
}

node::~node() {
    delete name;
}

void node::display() {
    cout << "   Name: " << *name << "   IP address: ";
    my_address.display();
}

int node::amIThisComputer(IPAddress a) {
    if(my_address.sameAddress(a))
        return 1;
    else
        return 0;
}

int node::myType() {
    return 0;
}

IPAddress node::myAddress() {
    return my_address;
}

//*****************************************//
// laptop class functions

laptop::laptop(string n, IPAddress a) : node(n,a)  {
    incoming = outgoing = NULL;
    my_server.parse("0.0.0.0");
}

int laptop::myType() {
    return LAPTOP;
}

void laptop::initiateDatagram(datagram* d) {
    outgoing = d;
}

void laptop::receiveDatagram(datagram* d) {
    incoming = d;
}

int  laptop::canAcceptConnection(int machine_type) {
    if(machine_type!=SERVER) return 0;
    return my_server.isNULL(); //we can only connect if the server is empty
}

void laptop::connect(IPAddress x, int machine_type) {
    if(machine_type==SERVER) my_server = x;
}

int laptop::canReceiveDatagram() {
    return (incoming==NULL);
}

void laptop::transferDatagram() {
    int i;
    extern node* network[MAX_MACHINES];
    
    if(outgoing==NULL) return;
    if(my_server.isNULL()) return;
    for (i = 0; i < MAX_MACHINES; i++) {
        if (network[i] != NULL) {
            if (network[i]->amIThisComputer(my_server))
                break;
        }
    }
    network[i]->receiveDatagram(outgoing);
    outgoing = NULL;
}

void laptop::display() {
    
    cout << "Laptop computer:  ";
    node::display();
    
    if(my_server.isNULL()) {
        cout << "\n    Not connected to any server.\n";
    }
    else {
        cout << "\nConnected to ";
        my_server.display();
        cout << "\n";
    }
    
    cout << "\n   Incoming datagram:  ";
    if(incoming==NULL) cout << "No datagram.";
    else {cout << "\n";  incoming->display(); }
    
    cout << "\n   Outgoing datagram:  ";
    if(outgoing==NULL) cout << "No datagram.";
    else {cout << "\n"; outgoing->display(); }
    
}

void laptop::consumeDatagram() {
    incoming = NULL;
}


/**************new*************/
laptop::~laptop() {
    if (incoming != NULL)
        delete incoming;
    if (outgoing != NULL)
        delete outgoing;
}
/**************new*************/

//*****************************************//
// server class functions

server::server(string n, IPAddress a) : node(n,a)  {
    number_of_laptops = number_of_wans = 0;
    dlist = new msg_list;
}


int server::myType() {
    return SERVER;
}

int  server::canAcceptConnection(int machine_type) {
    if(machine_type==LAPTOP)
        return (number_of_laptops<8);
    else if(machine_type==WAN_MACHINE)
        return (number_of_wans<4);
    return 0;
}

void server::connect(IPAddress x, int machine_type) {
    if(machine_type==LAPTOP)
        laptop_list[number_of_laptops++] = x;
    else if(machine_type==WAN_MACHINE)
        WAN_list[number_of_wans++] = x;
}

void server::receiveDatagram(datagram* d) {
    dlist->append(d);
}

void server::display() {
    int i;
    
    cout << "Server computer:  ";
    node::display();
    
    cout << "\n   Connections to laptops: ";
    if(number_of_laptops==0) cout << "    List is empty.";
    else for(i=0; i<number_of_laptops; i++) {
        cout << "\n      Laptop " << i+1 << ":   ";
        laptop_list[i].display();
    }
    cout << "\n\n   Connections to WANs:    ";
    if(number_of_wans==0) cout << "    List is empty.";
    else for(i=0; i<number_of_wans; i++) {
        cout << "\n         WAN " << i+1 << ":   ";
        WAN_list[i].display();
    }
    
    cout << "\n\n   Message list:\n";
    dlist->display();
    
    cout << "\n\n";
    
}

void server::transferDatagram() {
    extern node* network[MAX_MACHINES];
    datagram *dg;  IPAddress dest;  int octad;
    msg_list *tmp;
    int i, j, k, m, dist, msg_processed;
    
    tmp = new msg_list;
    
    dg = dlist->returnFront();
    while(dg!=NULL) {
        dest = dg->destinationAddress();
        msg_processed = 0;
        octad = dest.firstOctad();
        
        if(octad == my_address.firstOctad()) {
            // The destination is a laptop on this local area network
            for(i=0; (i<number_of_laptops) && (msg_processed==0); i++) {
                if(laptop_list[i].sameAddress(dest)) {
                    msg_processed = 1;
                    //j = findMachine(dest);
                    for (j = 0; j < MAX_MACHINES; j++) {
                        if (network[j] != NULL && network[j]->amIThisComputer(dest))
                            break;
                    }
                    if(j==MAX_MACHINES) tmp->append(dg);
                    else if( ((laptop *)network[j])->canReceiveDatagram() )
                        network[j]->receiveDatagram(dg);
                    else 	tmp->append(dg);
                }
            }
            msg_processed = 1;
        }
        
        if(msg_processed==0) {
            // Message does not belong on this local area network.  Send it to a WAN.
            dist = 257;
            k = -1;
            for(i=0; i<number_of_wans; i++) {
                //j = findMachine(WAN_list[i]);
                for (j = 0; j < MAX_MACHINES; j++) {
                    if (network[j] != NULL && network[j]->amIThisComputer(WAN_list[i]))
                        break;
                }
                //if(j>=0) {
                if (j!=MAX_MACHINES) {
                    m = octad - (network[j]->myAddress()).firstOctad();
                    if(m<0) m = -m;
                    if(m<dist){
                        dist = m;
                        k = j;
                    }
                }
            }
            // At this point, k is the target machine.
            if(k>=0) {
                network[k]->receiveDatagram(dg);
            }
            else	 tmp->append(dg);
        }
        dg = dlist->returnFront();
    }
    delete dlist;
    dlist = tmp;
}
/**************new*************/
server::~server() {
    dlist->deleteList();
}
/**************new*************/

//*****************************************//
// WAN class functions

WAN::WAN(string n, IPAddress a) : node(n,a)  {
    number_of_servers = number_of_wans = 0;
    dlist = new msg_list;
}

int WAN::myType() {
    return WAN_MACHINE;
}

int  WAN::canAcceptConnection(int machine_type) {
    if(machine_type==SERVER)
        return (number_of_servers<4);
    else if(machine_type==WAN_MACHINE)
        return (number_of_wans<4);
    
    return 0;
}

void WAN::connect(IPAddress x, int machine_type) {
    if(machine_type==SERVER)
        server_list[number_of_servers++] = x;
    else if(machine_type==WAN_MACHINE)
        WAN_list[number_of_wans++] = x;
}

void WAN::receiveDatagram(datagram* d) {
    dlist->append(d);
}


void WAN::display() {
    int i;
    
    cout << "WAN computer:  ";
    node::display();
    
    cout << "\n   Connections to servers: ";
    if(number_of_servers==0) cout << "    List is empty.";
    else for(i=0; i<number_of_servers; i++) {
        cout << "\n      Server " << i+1 << ":   ";
        server_list[i].display();
    }
    cout << "\n\n   Connections to WANs:    ";
    if(number_of_wans==0) cout << "    List is empty.";
    else for(i=0; i<number_of_wans; i++) {
        cout << "\n         WAN " << i+1 << ":   ";
        WAN_list[i].display();
    }
    
    cout << "\n\n   Message list:\n";
    dlist->display();
    
    cout << "\n\n";
    
}

void WAN::transferDatagram() {
    extern node* network[MAX_MACHINES];
    
    datagram *dg;  IPAddress dest;  int octad;
    msg_list *tmp;
    int i, j, k, m, dist, msg_processed;
    
    tmp = new msg_list;
    
    dg = dlist->returnFront();
    while(dg!=NULL) {
        dest = dg->destinationAddress();
        msg_processed = 0;
        octad = dest.firstOctad();
        
        // First check if the target is on a LAN connected to this WAN
        for(i=0; (i<number_of_servers) && (msg_processed==0); i++) {
            if( octad==server_list[i].firstOctad() ) {
                msg_processed = 1;
                //j = findMachine(server_list[i]);
                for (j = 0; j < MAX_MACHINES; j++) {
                    if (network[j] != NULL && network[j]->amIThisComputer(server_list[i]))
                        break;
                }
                if(j==MAX_MACHINES) tmp->append(dg);
                else 	network[j]->receiveDatagram(dg);
            }
        }
        
        if(msg_processed==0) {
            // Did not find the target local area network.  Send it to a WAN.
            dist = 257;
            k = -1;
            for(i=0; i<number_of_wans; i++) {
                //j = findMachine(WAN_list[i]);
                for (j = 0; j < MAX_MACHINES; j++) {
                    if (network[j] != NULL && network[j]->amIThisComputer(WAN_list[i]))
                        break;
                }
                //if(j>=0) {
                if (j!=MAX_MACHINES) {
                    m = octad - (network[j]->myAddress()).firstOctad();
                    if(m<0) m = -m;
                    if(m<dist){
                        dist = m;
                        k = j;
                    }
                }
            }
            // At this point, k is the target machine.
            if(k>=0) {
                network[k]->receiveDatagram(dg);
            }
            else	 tmp->append(dg);
        }
        
        dg = dlist->returnFront();
    }
    
    delete dlist;
    dlist = tmp;
}

/**************new*************/
WAN::~WAN() {
    dlist->deleteList();
}
/**************new*************/





















