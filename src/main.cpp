#include <iostream>
#include "pdu.h"

using namespace std;


int main()
{
    PDU pdu("0791839001446010040C9183904517829100000111726115318005C8329BFD06");
    cout << "PDU: "<< pdu.getPDU() << endl;
    
    if (!pdu.split())
        cout << "Split error: " << pdu.getLastError() << endl;
    else
    {
        cout << "PDU SMSC: " << pdu.getSMSC() << endl;
        cout << "PDU Sender Address Type: " << pdu.getSenderAddressType() << endl;
        cout << "PDU Sender: " << pdu.getSender() << endl;
        cout << "PDU Date: " << pdu.getDate() << endl;
        cout << "PDU Time: " << pdu.getTime() << endl;
        cout << "PDU Message: " << pdu.getText() << endl;
        cout << "PDU UDH Type: " << pdu.getUDHType() << endl;
        cout << "PDU UDH Data: " << pdu.getUDHData() << endl;
    }

    return 0;
}

