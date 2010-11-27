/* 
Based on SMS Server Tools 3
Copyright (C) 2006- Keijo Kasvi
http://smstools3.kekekasvi.com/

This program is free software unless you got it under another license directly
from the author. You can redistribute it and/or modify it under the terms of
the GNU General Public License as published by the Free Software Foundation.
Either version 2 of the License, or (at your option) any later version.
*/

#include "pdu.h"
#include <ctype.h>
#include <string.h>
#include <iostream>
#include <stdio.h>

using namespace std;


// Global constants
const int max_smsc_addr_len = 30;
const int max_addr_len = 50;
const int size_udh_data = 500;

// Helper methods

// Converts an octet to a 8-Bit value
int octet2bin(const char* octet) 
{
    int result = 0;
    if (octet[0] > 57)
        result += octet[0] - 55;
    else
        result += octet[0] - 48;
    result = result << 4;
    if (octet[1] > 57)
        result += octet[1] - 55;
    else
        result += octet[1] - 48;
    return result;
}

// Converts an octet to a 8bit value,
// returns < in case of error.
int octet2bin_check(const char *octet)
{
    if (octet[0] == 0)
        return -1;
    if (octet[1] == 0)
        return -2;
    if (!isxdigit(octet[0]))
        return -3;
    if (!isxdigit(octet[1]))
        return -4;
    return octet2bin(octet);
}

/* Swap every second character */
void swapchars(string& str) 
{
    for (size_t i = 0; i < str.length(); i += 2)
    {
        char c = str[i];
        str[i] = str[i + 1];
        str[i + 1] = c;
    }
}

int pdu2text(char *pdu, char *text, int *text_length, int *expected_length,
             int with_udh, char *udh, char *udh_type, int *errorpos) 
{
    int bitposition;
    int byteposition;
    int byteoffset;
    int charcounter;
    int bitcounter;
    int septets;
    int octets;
    int udhsize;
    int octetcounter;
    int skip_characters = 0;
    char c;
    char binary = 0;
    int i;
    int result;

    if (udh) 
        *udh = 0;
    if (udh_type)
        *udh_type = 0;

    if ((septets = octet2bin_check(pdu)) < 0)
    {
        if (errorpos)
            *errorpos = -1 * septets -3;
        return (septets >= -2)? -2: -1;
    }

    if (with_udh)
    {
        // copy the data header to udh and convert to hex dump
        // There was at least one octet and next will give an error if there is no more data:
        if ((udhsize = octet2bin_check(pdu +2)) < 0)
        {
            if (errorpos)
                *errorpos = -1 * udhsize -3 +2;
            return (udhsize >= -2)? -2: -1;
        }

        i = 0;
        result = -1;
        for (octetcounter = 0; octetcounter < udhsize + 1; octetcounter++)
        {
            if (octetcounter *3 +3 >= size_udh_data)
            {
                i = octetcounter *2 +2;
                result = -2;
                break;
            }
            udh[octetcounter*3]=pdu[(octetcounter<<1)+2];
            if (!isxdigit(udh[octetcounter *3]))
            {
                i = octetcounter *2 +2;
                if (!udh[octetcounter *3])
                    result = -2;
                break;
            }
            udh[octetcounter*3+1]=pdu[(octetcounter<<1)+3];
            if (!isxdigit(udh[octetcounter *3 +1]))
            {
                i = octetcounter *2 +3;
                if (!udh[octetcounter *3 +1])
                    result = -2;
                break;
            }
            udh[octetcounter *3 +2] = ' ';
            udh[octetcounter *3 +3] = 0;
        }

        if (i)
        {
            if (errorpos)
            *errorpos = i;
            return result;
        }

        /*TODO
        if (udh_type)
        if (explain_udh(udh_type, pdu +2) < 0)
            if (strlen(udh_type) +7 < SIZE_UDH_TYPE)
                sprintf(strchr(udh_type, 0), "%sERROR", (*udh_type)? ", " : "");*/

        // Calculate how many text charcters include the UDH.
        // After the UDH there may follow filling bits to reach a 7bit boundary.
        skip_characters=(((udhsize+1)*8)+6)/7;
    }

    if (expected_length)
        *expected_length = septets - skip_characters;

    // Convert from 8-Bit to 7-Bit encapsulated in 8 bit 
    // skipping storing of some characters used by UDH.
    // 3.1beta7: Simplified handling to allow partial decodings to be shown.
    octets = (septets *7 +7) /8;     
    bitposition = 0;
    octetcounter = 0;
    for (charcounter = 0; charcounter < septets; charcounter++)
    {
        c = 0;
        for (bitcounter = 0; bitcounter < 7; bitcounter++)
        {
            byteposition = bitposition /8;
            byteoffset = bitposition %8;
            while (byteposition >= octetcounter && octetcounter < octets)
            {
                if ((i = octet2bin_check(pdu +(octetcounter << 1) +2)) < 0)
                {
                    if (errorpos)
                    {
                        *errorpos = octetcounter *2 +2;
                        if (i == -2 || i == -4)
                            (*errorpos)++;
                    }
                    if (text_length)
                        *text_length = charcounter -skip_characters;
                    return (i >= -2)? -2: -1;
                }
                binary = i;
                octetcounter++;
            }
            if (binary & (1 << byteoffset))
                c = c | 128;
            bitposition++;
            c = (c >> 1) & 127; // The shift fills with 1, but 0 is wanted.
        }
        if (charcounter >= skip_characters)
            text[charcounter -skip_characters] = c; 
    }

    if (text_length)
        *text_length = charcounter -skip_characters;

    if (charcounter -skip_characters >= 0)
        text[charcounter -skip_characters] = 0;
    return charcounter -skip_characters;
}

int pdu2text0(char *pdu, char *text)
{
    return pdu2text(pdu, text, 0, 0, 0, 0, 0, 0);
}


PDU::PDU(const string& pdu)
    : m_pdu(pdu)
{}

bool PDU::readSMSC()
{
    int length;
    
    if ((length = octet2bin_check(m_pdu_ptr)) < 0)
    {
        m_err_msg = "Invalid character(s) in first octet";
        return false;
    }

    if (length == 0)
    {
        m_pdu_ptr += 2;
        return true;
    }

    if (length < 2 || length > max_smsc_addr_len)
    {
        m_err_msg = "Invalid sender SMSC address length";
        return false;
    }
    
    length = length * 2 - 2;
    // No padding because the given value is number of octets.
    if (m_pdu.length() < (size_t)(length + 4))
    {
        m_err_msg = "Reading SMSC address: PDU is too short";
        return false;
    }
    
    m_pdu_ptr += 2;
    // Read SMSC address type
    int addr_type = octet2bin_check(m_pdu_ptr);
    if (addr_type < 0)
    {
        m_err_msg = "Invalid sender SMSC address type";
        return false;
    }
    if (addr_type < 0x80)
    {
        m_err_msg = "Missing bit 7 in sender SMSC address type";
        return false;
    }
    
    // Read SMSC address
    m_pdu_ptr += 2;
    m_smsc = string(m_pdu_ptr, (size_t)length);
    swapchars(m_smsc); 
    
    // Does any SMSC use alphanumeric number?
    if ((addr_type & 112) == 80)
    {
        // There can be only hex digits from the original PDU.
        // The number itself is wrong in this case.
        for (size_t i = 0; i < m_smsc.length(); i++)
        {
            if (!isxdigit(m_smsc[i]))
            {
                m_err_msg = "Invalid character(s) in alphanumeric SMSC address";
                return false;
            }
        }
    }
    else
    {
        // Last character is allowed as F (and dropped) but all other non-numeric will produce an error:
        if (m_smsc[length -1] == 'F')
            m_smsc.resize(length - 1);
        for (size_t i = 0; i < m_smsc.length(); i++)
        {
            if (!isdigit(m_smsc[i]))
            {
                m_err_msg = "Invalid character(s) in numeric SMSC address";
                return false;
            }
        }
    }
    
    m_pdu_ptr += length;
    return true;
}

bool PDU::split()
{   
    // Patch for Wavecom SR memory reading:
    if (int(m_pdu.find("000000FF00")) == 0)
    {
        m_pdu.resize(8);// or m_pdu = m_pdu.substr(0, 8);
        while (m_pdu.length() < 52)
            m_pdu += "00";
    }
    // ------------------------------------

    m_sender = "";
    m_date = "";
    m_time = "";
    m_text = "";
    m_smsc = ""; 
    m_alphabet = ISO;
    
    if (m_pdu.length() < 2)
    {
        m_err_msg = "PDU is too short";
        return false;
    }
    
    m_pdu_ptr = m_pdu.c_str();

    if (!readSMSC())
        return false;
        
    if (strlen(m_pdu_ptr) < 2)
    {
        m_err_msg = "Reading First octet of the SMS-DELIVER PDU: PDU is too short";
        return false;
    }
    
    int tmp;
    if ((tmp = octet2bin_check(m_pdu_ptr)) < 0)
    {
         m_err_msg = "Reading First octet of the SMS-DELIVER PDU: Invalid character(s)";
         return false;
    }
    
    // Unused bits 3 and 4 should be zero, failure with this produces a warning:
/*    if (tmp & 0x18)
        cout << "Unused bits 3 and 4 are used in the first octet of the SMS-DELIVER PDU." << endl;
    
    if (tmp & 0x40) // Is UDH bit set?
        with_udh = true;
    
    if (tmp & 0x20) // Is status report going to be returned to the SME?
        report = true;*/
    
    int type = tmp & 3;
    if (type == 0) // SMS Deliver
    {
        m_pdu_ptr += 2;
        if (!splitDeliver())
            return false;
    }
    else if (type == 2) // Status Report
    {}
    else if (type == 1) // Sent message
    {}
    else
    {
        m_err_msg = "Unsupported type";
        return false;
    }

    return true;
}

bool PDU::splitDeliver()
{
    if (strlen(m_pdu_ptr) < 4)
    {
        m_err_msg = "Reading address length and address type: PDU is too short";
        return false;
    }
    
    int length = octet2bin_check(m_pdu_ptr);
    if (length < 0 || length > max_addr_len)
    {
        m_err_msg = "Invalid sender address length";
        return false;
    }
    if (length == 0)
        m_pdu_ptr += 4;
    else
    {
        int padding = length % 2;
        m_pdu_ptr += 2;
        int addr_type = explainAddressType(m_pdu_ptr, 0);
        if (addr_type < 0)
        {
            m_err_msg = "Invalid sender address type";
            return false;
        }
        if (addr_type < 0x80)
        {
            m_err_msg = "Missing bit 7 in sender address type";
            return false;
        }
        
        m_pdu_ptr += 2;
        if ((addr_type & 112) == 80) // Sender is alphanumeric
        {
            if (strlen(m_pdu_ptr) < (size_t)(length + padding))
            {
                m_err_msg = "Reading sender address (alphanumeric): PDU is too short";
                return false;
            }
            char tmpsender[100];
            char sender[100];
            snprintf(tmpsender, length + padding + 3, "%02X%s", length * 4 / 7, m_pdu_ptr);
            if (pdu2text0(tmpsender, sender) < 0)
            {
                m_err_msg = "Reading alphanumeric sender address: Invalid character(s)";
                return false;
            }
            m_sender = sender;
        }
        else // Sender is numeric
        {
            m_sender = string(m_pdu_ptr, length + padding);
            swapchars(m_sender);
           
            int end = length + padding - 1;
            if (padding)
            {
                if (m_sender[end] != 'F')
                    cout << "Length of numeric sender address is odd, but not terminated with 'F'." << endl;
                else
                    m_sender.resize(end);
            }
            else
            {
                if (m_sender[end] == 'F')
                {
                    cout << "Length of numeric sender address is even, but still was terminated with 'F'." << endl;
                    m_sender.resize(end);
                }
            }
            
            for (size_t i = 0; i < m_sender.length(); i++)
            {
                if (!isdigit(m_sender[i]))
                {
                    cout << "Invalid character(s) in sender address." << endl;
                    break;
                }
            }
        }
    }
    
    // pdu.c:1129
    
    return true;
}

int PDU::explainAddressType(const char *octet_char, int octet_int)
{
    int result;
    const char *p = "reserved";
    
    if (octet_char)
        result = octet2bin_check(octet_char);
    else
        result = octet_int;

    if (result != -1)
    {
        switch ((result & 0x70) >> 4)
        {
            case 0: p = "unknown"; break;
            case 1: p = "international"; break;
            case 2: p = "national"; break;
            case 3: p = "network specific"; break;
            case 4: p = "subsciber"; break;
            case 5: p = "alphanumeric"; break;
            case 6: p = "abbreviated"; break;
        }
        
        m_sender_addr_type = p;
        
        switch (result & 0x0F)
        {
            case 0: p = "unknown"; break;
            case 1: p = "ISDN/telephone"; break;
            case 3: p = "data"; break;
            case 4: p = "telex"; break;
            case 8: p = "national"; break;
            case 9: p = "private"; break;
            case 10: p = "ERMES"; break;
        }
        m_sender_addr_type += ", ";
        m_sender_addr_type += p;
    }
    return result;
}

