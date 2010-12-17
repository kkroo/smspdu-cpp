#include <stdio.h>
#include <string.h>
#include "pdu.h"

using namespace std;


int main()
{
    printf("Parsing SMS Deliver:\n");
    PDU pdu("0791839001446010040C9183904517829100000111726115318005C8329BFD06");
        
    if (!pdu.parse())
    {
        printf("PDU parsing failed. Error: %s\n", pdu.getError());
        return -1;
    }
    printf("PDU: %s\n", pdu.getPDU());
    printf("SMSC: %s\n", pdu.getSMSC());
    printf("Sender: %s\n", pdu.getNumber());
    printf("Sender Number Type: %s\n", pdu.getNumberType());
    printf("Date: %s\n", pdu.getDate());
    printf("Time: %s\n", pdu.getTime());
    printf("UDH Type: %s\n", pdu.getUDHType());
    printf("UDH Data: %s\n", pdu.getUDHData());
    printf("Message: %s\n", pdu.getMessage());

    printf("Generating PDU:\n");
    PDU pdu2;
    
    pdu2.setMessage("Привет, мир!");
    pdu2.setNumber("380911234567");
    pdu2.setSMSC("+380910440601");
    pdu2.setAlphabet(PDU::UCS2);
    
    pdu2.generate();
    printf("PDU: %s\n", pdu2.getPDU());
    printf("PDU Length: %d\n", pdu2.getMessageLen());

    return 0;
}

