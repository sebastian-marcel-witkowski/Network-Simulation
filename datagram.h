#ifndef DATAGRAM
#define DATAGRAM

#include <iostream>
#include <string>

#include "definitions.h"

using namespace std;

class IPAddress {
	private:
		unsigned int a[4];
	
	public:
        IPAddress();
		int parse(string);
		void display() const;
		int	sameAddress(IPAddress x);
        int	isNULL();
        /************new**********/
        int firstOctad();
        /************new**********/
};


class datagram {
	private:
		IPAddress src, dest;
		int length;
		string msg;
	
	public:
		void makeDatagram(IPAddress s, IPAddress d, string m);
		void display() const;
        /************new**********/
        IPAddress destinationAddress();
        /************new**********/
};


#endif