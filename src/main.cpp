#include <iostream>
#include "pdu.h"

using namespace std;


int main()
{
    PDU pdu("0791839001446010040C9183904510229100000111726115318005C8329BFD06");
    cout << "PDU: "<< pdu.getPDU() << endl;
    
    if (!pdu.split())
        cout << "Split error: " << pdu.getLastError() << endl;
    else
    {
        cout << "PDU SMSC: " << pdu.getSMSC() << endl;
        cout << "PDU Sender Address Type: " << pdu.getSenderAddressType() << endl;
        cout << "PDU Sender: " << pdu.getSender() << endl;
    }

    return 0;
}

