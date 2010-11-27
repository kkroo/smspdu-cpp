/* 
Based on SMS Server Tools 3
Copyright (C) 2006- Keijo Kasvi
http://smstools3.kekekasvi.com/

This program is free software unless you got it under another license directly
from the author. You can redistribute it and/or modify it under the terms of
the GNU General Public License as published by the Free Software Foundation.
Either version 2 of the License, or (at your option) any later version.
*/

#ifndef PDU_H
#define PDU_H

#include <string>

using std::string;


class PDU {
public:
    enum NumerFormat {
        NF_UNKNOWN = 129,
        NF_INTERNATIONAL = 145,
        NF_NATIONAL = 161
    };
    
    enum Alphabet {
        GSM = -1,
        ISO = 0,
        BINARY = 1,
        UCS2 = 2
    };
    
    enum Mode {
        OLD,
        NEW
    };
    
    // Constructors/Destructors
    PDU(const string& pdu);
    virtual ~PDU() {}
    
    // Getters
    inline string getPDU() { return m_pdu; }
    inline string getSender() { return m_sender; }
    inline string getText() { return m_text; }
    inline string getSMSC() { return m_smsc; }
    inline string getLastError() { return m_err_msg; }
    inline string getSenderAddressType() { return m_sender_addr_type; }
    
    // Staff
    bool readSMSC();
    bool splitDeliver();
    bool split();
    int explainAddressType(const char *octet_char, int octet_int);
    

private:
    NumerFormat m_number_frmt;
    string m_pdu;
    const char *m_pdu_ptr; // Pointer to chars into m_pdu. For internal use only
    string m_err_msg;
    string m_sender;
    string m_sender_addr_type;
    string m_text;
    string m_date;
    string m_time;
    Alphabet m_alphabet;
    string m_smsc; 
    Mode m_mode;
    
};

#endif

