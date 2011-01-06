/* 
Copyright (C) 2010- Alexander Chudnovets <effractor@gmail.com>

Based on SMS Server Tools 3
Copyright (C) 2006- Keijo Kasvi
http://smstools3.kekekasvi.com/

This program is free software unless you got it under another license directly
from the author. You can redistribute it and/or modify it under the terms of
the GNU General Public License as published by the Free Software Foundation.
Either version 2 of the License, or (at your option) any later version.
*/

#include <stdio.h>
#include <string.h>
#include "pdu.h"

using namespace std;


int main()
{
    printf("Parsing SMS Deliver:\n");
    PDU pdu("079183600300008006070C9183903113476111103091244280111030912482800000");
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

