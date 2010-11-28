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
#include <stdlib.h>

using namespace std;


// Global constants
const int max_smsc_addr_len = 30;
const int max_addr_len = 50;
const int size_udh_data = 500;
const int maxtext = 39016;
const int size_udh_type = 4096;

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

// TODO: remove it!
char *strcpyo(char *dest, const char *src)
{
    size_t i;

    for (i = 0; src[i] != '\0'; i++)
        dest[i] = src[i];

    dest[i] = '\0';

    return dest;
}
// Returns a length of udh (including UDHL), -1 if error.
// pdu is 0-terminated ascii(hex) pdu string with
// or without spaces.
int explain_udh(char *udh_type, const char *pdu)
{
    int udh_length;
    int idx;
    char *pdu_ptr;
    char *p;
    const char *p1;
    int i;
    char tmp[512];
    char buffer[1024];

    *udh_type = 0;
    if (strlen(pdu) >= sizeof(buffer))
        return -1;
    strcpy(buffer, pdu);
    while ((p = strchr(buffer, ' ')))
        strcpyo(p, p +1);
    
    if ((udh_length = octet2bin_check(buffer)) < 0)
        return -1;
    udh_length++;
    if ((size_t)(udh_length *2) > strlen(buffer))
        return -1;
    sprintf(udh_type, "Length=%i", udh_length);
    idx = 1;
    while (idx < udh_length)
    {
        pdu_ptr = buffer + idx * 2;
        p = NULL;
        i = octet2bin_check(pdu_ptr);
        switch (i)
        {
        case -1: return -1;

        // 3GPP TS 23.040 version 6.8.1 Release 6 - ETSI TS 123 040 V6.8.1 (2006-10)
        case 0x00: p1 = "Concatenated short messages, 8-bit reference number"; break;
        case 0x01: p1 = "Special SMS Message Indication"; break;
        case 0x02: p1 = "Reserved"; break;
        //case 0x03: p = "Value not used to avoid misinterpretation as <LF> character"; break;
        case 0x04: p1 = "Application port addressing scheme, 8 bit address"; break;
        case 0x05: p1 = "Application port addressing scheme, 16 bit address"; break;
        case 0x06: p1 = "SMSC Control Parameters"; break;
        case 0x07: p1 = "UDH Source Indicator"; break;
        case 0x08: p1 = "Concatenated short message, 16-bit reference number"; break;
        case 0x09: p1 = "Wireless Control Message Protocol"; break;
        case 0x0A: p1 = "Text Formatting"; break;
        case 0x0B: p1 = "Predefined Sound"; break;
        case 0x0C: p1 = "User Defined Sound (iMelody max 128 bytes)"; break;
        case 0x0D: p1 = "Predefined Animation"; break;
        case 0x0E: p1 = "Large Animation (16*16 times 4 = 32*4 =128 bytes)"; break;
        case 0x0F: p1 = "Small Animation (8*8 times 4 = 8*4 =32 bytes)"; break;
        case 0x10: p1 = "Large Picture (32*32 = 128 bytes)"; break;
        case 0x11: p1 = "Small Picture (16*16 = 32 bytes)"; break;
        case 0x12: p1 = "Variable Picture"; break;
        case 0x13: p1 = "User prompt indicator"; break;
        case 0x14: p1 = "Extended Object"; break;
        case 0x15: p1 = "Reused Extended Object"; break;
        case 0x16: p1 = "Compression Control"; break;
        case 0x17: p1 = "Object Distribution Indicator"; break;
        case 0x18: p1 = "Standard WVG object"; break;
        case 0x19: p1 = "Character Size WVG object"; break;
        case 0x1A: p1 = "Extended Object Data Request Command"; break;
        case 0x20: p1 = "RFC 822 E-Mail Header"; break;
        case 0x21: p1 = "Hyperlink format element"; break;
        case 0x22: p1 = "Reply Address Element"; break;
        case 0x23: p1 = "Enhanced Voice Mail Information"; break;
        }

        if (!p1)
        {
            if (i >= 0x1B && i <= 0x1F)
                p1 = "Reserved for future EMS features";
            else if (i >= 0x24 && i <= 0x6F)
                p1 = "Reserved for future use";
            else if (i >= 0x70 && i <= 0x7F)
                p1 = "(U)SIM Toolkit Security Headers";
            else if (i >= 0x80 && i <= 0x9F)
                p1 = "SME to SME specific use";
            else if (i >= 0xA0 && i <= 0xBF)
                p1 = "Reserved for future use";
            else if (i >= 0xC0 && i <= 0xDF)
                p1 = "SC specific use";
            else if (i >= 0xE0 && i <= 0xFF)
                p1 = "Reserved for future use";
        }
    
        if (!p1)
            p1 = "unknown";
        sprintf(tmp, ", [%.2s]%s", pdu_ptr, p1);
        if (strlen(udh_type) + strlen(tmp) >= (size_t)size_udh_type)
            return -1;
        sprintf(strchr(udh_type, 0), "%s", tmp);

        // Next octet is length of data:
        if ((i = octet2bin_check(pdu_ptr + 2)) < 0)
            return -1;
        if ((size_t)(i *2) > strlen(pdu_ptr + 4))
            return -1;
        idx += i + 2;
        if (idx > udh_length)
            return -1; // Incorrect UDL or length of Information Element.
    }

    return udh_length;
}

int pdu2text(const char *pdu, char *text, int *text_length, int *expected_length,
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

        if (udh_type)
            if (explain_udh(udh_type, pdu + 2) < 0)
                if (strlen(udh_type) + 7 < (size_t)size_udh_type)
                    sprintf(strchr(udh_type, 0), "%sERROR", (*udh_type)? ", " : "");

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

// Converts a PDU string to binary. Return -1 if there is a PDU error, -2 if PDU is too short.
// Version > 3.0.9, > 3.1beta6 handles also udh.
int pdu2binary(const char* pdu, char* binary, int *data_length, 
               int *expected_length, int with_udh, char *udh, char *udh_type, 
               int *errorpos)
{
    int octets;
    int octetcounter;
    int i;
    int udhsize = 0;
    int skip_octets = 0;
    int result;

    *udh = 0;
    *udh_type = 0;

    if ((octets = octet2bin_check(pdu)) < 0)
    {
        *errorpos = -1 * octets -3;
        return (octets >= -2)? -2: -1;
    }

    if (with_udh)
    {
        // copy the data header to udh and convert to hex dump
        // There was at least one octet and next will give an error if there is no more data:
        if ((udhsize = octet2bin_check(pdu +2)) < 0)
        {
            *errorpos = -1 * udhsize -3 +2;
            return (udhsize >= -2)? -2: -1;
        }

        i = 0;
        result = -1;
        for (octetcounter = 0; octetcounter < udhsize +1; octetcounter++)
        {
            if (octetcounter *3 +3 >= size_udh_data)
            {
                i = octetcounter *2 +2;
                result = -2;
                break;
            }
            udh[octetcounter *3] = pdu[(octetcounter << 1) +2];
            if (!isxdigit(udh[octetcounter *3]))
            {
                i = octetcounter *2 +2;
                if (!udh[octetcounter *3])
                    result = -2;
                break;
            }
            udh[octetcounter *3 +1] = pdu[(octetcounter << 1) +3];
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
            *errorpos = i;
            return result;
        }

        if (udh_type)
            if (explain_udh(udh_type, pdu + 2) < 0)
                if (strlen(udh_type) + 7 < (size_t)size_udh_type)
                    sprintf(strchr(udh_type, 0), "%sERROR", (*udh_type)? ", " : "");

        skip_octets = udhsize + 1;
    }

    *expected_length = octets -skip_octets;

    for (octetcounter = 0; octetcounter < octets - skip_octets; octetcounter++)
    {
        if ((i = octet2bin_check(pdu +(octetcounter << 1) +2 +(skip_octets *2))) < 0)
        {
            *errorpos = octetcounter *2 +2 +(skip_octets *2);
            if (i == -2 || i == -4)
                (*errorpos)++;
            *data_length = octetcounter;
            return (i >= -2)? -2: -1;
        }
        else
            binary[octetcounter] = i;
    }

    if (octets -skip_octets >= 0)
        binary[octets -skip_octets] = 0;
    *data_length = octets -skip_octets;
    return octets -skip_octets;
}

// PDU methods
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
    m_alphabet = 0;
    m_flash = false;
    m_with_udh = false;
    m_udh_type = "";
    m_udh_data = "";
    m_is_statusreport = false;
    
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
    if (tmp & 0x18)
        cout << "Unused bits 3 and 4 are used in the first octet of the SMS-DELIVER PDU." << endl;
    
    if (tmp & 0x40) // Is UDH bit set?
        m_with_udh = true;
    
    /*if (tmp & 0x20) // Is status report going to be returned to the SME?
        report = true;*/
    
    int type = tmp & 3;
    if (type == 0) // SMS Deliver
    {
        m_pdu_ptr += 2;
        if (!splitDeliver())
            return false;
    }
    else if (type == 2) // Status Report
    {
        m_pdu_ptr += 2;
        if(!splitStatusReport())
            return false;
        m_is_statusreport = true;
    }
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
    
    int padding = 0;
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
        padding = length % 2;
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
    
    m_pdu_ptr += length + padding;
    // Next there should be:
    // XX protocol identifier
    // XX data encoding scheme
    // XXXXXXXXXXXXXX time stamp, 7 octets
    // XX length of user data
    // ( XX... user data  )
    if (strlen(m_pdu_ptr) < 20)
    {
        m_err_msg = "Reading TP-PID, TP-DSC, TP-SCTS and TP-UDL: PDU is too short";
        return false;
    }
    // PID
    int byte_buf;
    if ((byte_buf = octet2bin_check(m_pdu_ptr)) < 0)
    {
        m_err_msg = "Invalid protocol identifier";
        return false;
    }
    if ((byte_buf & 0xF8) == 0x40)
        m_replace = (byte_buf & 0x07);
    
    // Alphabet
    m_pdu_ptr += 2;
    if ((byte_buf = octet2bin_check(m_pdu_ptr)) < 0)
    {
        m_err_msg = "Invalid data encoding scheme";
        return false;
    }
    m_alphabet = (byte_buf & 0x0C) >> 2;
    if (m_alphabet == 3)
    {
        // TODO: or should this be a warning? If so, GSM alphabet is then used as a default.
        m_err_msg = "Invalid alphabet in data encoding scheme: value 3 is not supported";
        return false;
    }
    if (m_alphabet == 0)
        m_alphabet = -1;
    
    // 3.1: Check if this message was a flash message:
    if (byte_buf & 0x10)
        if (!(byte_buf & 0x01))
            m_flash = true;
    
    // Date
    m_pdu_ptr += 2;
    char str_buf[100]; // TODO: replace it to strings?
    sprintf(str_buf, "%c%c-%c%c-%c%c", m_pdu_ptr[1], m_pdu_ptr[0], m_pdu_ptr[3],
            m_pdu_ptr[2], m_pdu_ptr[5], m_pdu_ptr[4]);
    if (!isdigit(str_buf[0]) || !isdigit(str_buf[1]) || !isdigit(str_buf[3]) 
        || !isdigit(str_buf[4]) || !isdigit(str_buf[6]) || !isdigit(str_buf[7]))
    {
        m_err_msg = "Invalid character(s) in date of Service Centre Time Stamp";
        return false;
    }
    else if (atoi(str_buf + 3) > 12 || atoi(str_buf + 6) > 31)
    {
        // Not a fatal error (?)
        //pdu_error(err_str, 0, Src_Pointer -full_pdu, 6, "Invalid value(s) in date of Service Centre Time Stamp: \"%s\"", date);
        // *date = 0;
        cout << "Invalid values(s) in date of Service Centre Time Stamp.";
    }
    m_date = str_buf;
    
    // Time
    m_pdu_ptr += 6;
    sprintf(str_buf, "%c%c:%c%c:%c%c", m_pdu_ptr[1], m_pdu_ptr[0], m_pdu_ptr[3],
            m_pdu_ptr[2], m_pdu_ptr[5], m_pdu_ptr[4]);
    if (!isdigit(str_buf[0]) || !isdigit(str_buf[1]) || !isdigit(str_buf[3]) 
        || !isdigit(str_buf[4]) || !isdigit(str_buf[6]) || !isdigit(str_buf[7]))
    {
        m_err_msg = "Invalid character(s) in time of Service Centre Time Stamp";
        return false;
    }
    else if (atoi(str_buf) > 23 || atoi(str_buf + 3) > 59 || atoi(str_buf + 6) > 59)
    {
        // Not a fatal error (?)
        //pdu_error(err_str, 0, Src_Pointer -full_pdu, 6, "Invalid value(s) in time of Service Centre Time Stamp: \"%s\"", time);
        // *time = 0;
        cout << "Invalid values(s) in time of Service Centre Time Stamp.";
    }
    m_time = str_buf;
    
    m_pdu_ptr += 6;
    // Time zone is not used but bytes are checked:
    if (octet2bin_check(m_pdu_ptr) < 0)
    {
        m_err_msg = "Invalid character(s) in Time Zone of Service Centre Time Stamp";
        return false;
    }
    
    m_pdu_ptr += 2;
    
    // TODO: clean it
    int result = 0;
    char message[maxtext]; message[0] = 0;
    int message_length;
    int expected_length;
    char udh_data[size_udh_data]; udh_data[0] = 0;
    char udh_type[size_udh_type]; udh_type[0] = 0;
    int errorpos = 0;
    int bin_udh = 1;    // UDH binary format flag
    
    printf("alpha[%d]\n", m_alphabet);
    if (m_alphabet <= 0)
    {
        if ((result = pdu2text(m_pdu_ptr, message, &message_length, 
                               &expected_length, (int)m_with_udh, udh_data, 
                               udh_type, &errorpos)) < 0)
        {
            m_err_msg = "Error while reading TP-UD (GSM text)";
            return false;
        }
    }
    else
    {
        // With binary messages udh is NOT taken from the PDU.
        int i = (int)m_with_udh;
        // 3.1.5: it should work now:
        if (bin_udh == 0)
            if (m_alphabet == 1)
                i = 0;
        if ((result = pdu2binary(m_pdu_ptr, message, &message_length, 
                                 &expected_length, i, udh_data, udh_type, 
                                 &errorpos)) < 0)
        {
            m_err_msg = "Error while reading TP-UD ";
            m_err_msg += (m_alphabet == 1) ? "binary" : "UCS2 text";
            return false;
        } 
        printf("[%s]\n", message);
    }
    
    m_text = message;
    m_udh_type = udh_type;
    m_udh_data = udh_data;
    
    return true;
}

// Subroutine for messages type 2 (Status Report)
// Input: 
// Src_Pointer points to the PDU string
// Output:
// sendr Sender
// date and time Date/Time-stamp
// result is the status value and text translation
bool PDU::splitStatusReport()
{
    const char *message_id_label = "Message_id:"; // Fixed title inside the status report body.
    const char *status_label = "Status:"; // Fixed title inside the status report body.

    char result[1024];
    result[0] = 0;
    strcat(result, "SMS STATUS REPORT\n");

    // There should be at least message-id, address-length and address-type:
    if (strlen(m_pdu_ptr) < 6)
    {
        m_err_msg = "Reading message id, address length and address type: PDU is too short";
        return false;
    }

    int messageid;
    if ((messageid = octet2bin_check(m_pdu_ptr)) < 0)
    {
        m_err_msg = "Invalid message id";
        return false;
    }
    sprintf(strchr(result, 0), "%s %i\n", message_id_label, messageid);
    
    // Get recipient address
    m_pdu_ptr += 2;
    int length = octet2bin_check(m_pdu_ptr);
    if (length < 1 || length > max_addr_len)
    {
        m_err_msg = "Invalid recipient address length";
        return false;
    }
    
    int padding = length % 2;
    m_pdu_ptr += 2;
    
    int addr_type = explainAddressType(m_pdu_ptr, 0);
    if (addr_type < 0)
    {
        m_err_msg = "Invalid recipient address type";
        return false;
    }
    if (addr_type < 0x80)
    {
        m_err_msg = "Missing bit 7 in recipient address type";
        return false;
    }
    
    char tmpsender[100];
    char sendr[100];
    m_pdu_ptr += 2;
    if ((addr_type & 112) == 80) // Sender is alphanumeric
    {  
        if (strlen(m_pdu_ptr) < (size_t)(length + padding))
        {
            m_err_msg = "While trying to read recipient address (alphanumeric)";
            return false;
        }
        
        snprintf(tmpsender, length + padding + 3, "%02X%s", length * 4 / 7, m_pdu_ptr);
        if (pdu2text0(tmpsender, sendr) < 0)
        {
            m_err_msg = "Reading alphanumeric recipient address: Invalid character(s)";
            return false;
        }
        m_sender = sendr;
    }
    else // Sender is numeric
    { 
        if (strlen(m_pdu_ptr) < (size_t)(length + padding))
        {
            m_err_msg = "Reading recipient address (numeric): PDU is too short";
            return false;
        }
        
        strncpy(sendr, m_pdu_ptr, length + padding);
        sendr[length + padding] = 0;
        m_sender = sendr;
        swapchars(m_sender);
        
        int end = length + padding -1;
        if (padding)
        {
            if (m_sender[end] != 'F')
                cout << "Length of numeric recipient address is odd, but not terminated with 'F'." << endl;
            else
                m_sender.resize(end);
        }
        else
        {
            if (m_sender[end] == 'F')
            {
                cout << "Length of numeric recipient address is even, but still was terminated with 'F'." << endl;
                m_sender.resize(end);
            }
        }
        for (size_t i = 0; i < m_sender.length(); i++)
            if (!isdigit(m_sender[i]))
            {
                // Not a fatal error (?)
                //pdu_error(err_str, 0, Src_Pointer -full_pdu, Length +padding, "Invalid character(s) in recipient address: \"%s\"", sendr);
                // *sendr = 0;
                cout << "Invalid character(s) in recipient address." << endl;
                break;
            }
    }
    
    m_pdu_ptr += length + padding;
    if (strlen(m_pdu_ptr) < 14)
    {
        m_err_msg = "While trying to read SMSC Timestamp: PDU is too short";
        return false;
    }
 /*           else
            {
              // get SMSC timestamp
              sprintf(date,"%c%c-%c%c-%c%c",Src_Pointer[1],Src_Pointer[0],Src_Pointer[3],Src_Pointer[2],Src_Pointer[5],Src_Pointer[4]);
              if (!isdigitc(date[0]) || !isdigitc(date[1]) || !isdigitc(date[3]) || !isdigitc(date[4]) || !isdigitc(date[6]) || !isdigitc(date[7]))
              {
                pdu_error(err_str, 0, Src_Pointer -full_pdu, 6, "Invalid character(s) in date of SMSC Timestamp: \"%s\"", date);
                *date = 0;
              }
              else if (atoi(date +3) > 12 || atoi(date +6) > 31)
              {
                // Not a fatal error (?)
                //pdu_error(err_str, 0, Src_Pointer -full_pdu, 6, "Invalid value(s) in date of SMSC Timestamp: \"%s\"", date);
                // *date = 0;
                add_warning(warning_headers, "Invalid value(s) in date of SMSC Timestamp.");
              }

              Src_Pointer += 6;
              sprintf(time,"%c%c:%c%c:%c%c",Src_Pointer[1],Src_Pointer[0],Src_Pointer[3],Src_Pointer[2],Src_Pointer[5],Src_Pointer[4]);
              if (!isdigitc(time[0]) || !isdigitc(time[1]) || !isdigitc(time[3]) || !isdigitc(time[4]) || !isdigitc(time[6]) || !isdigitc(time[7]))
              {
                pdu_error(err_str, 0, Src_Pointer -full_pdu, 6, "Invalid character(s) in time of SMSC Timestamp: \"%s\"", time);
                *time = 0;
              }
              else if (atoi(time) > 23 || atoi(time +3) > 59 || atoi(time +6) > 59)
              {
                // Not a fatal error (?)
                //pdu_error(err_str, 0, Src_Pointer -full_pdu, 6, "Invalid value(s) in time of SMSC Timestamp: \"%s\"", time);
                // *time = 0;
                add_warning(warning_headers, "Invalid value(s) in time of SMSC Timestamp.");
              }

              if (!(*err_str))
              {
                Src_Pointer += 6;
                // Time zone is not used but bytes are checked:
                if (octet2bin_check(Src_Pointer) < 0)
                  pdu_error(err_str, 0, Src_Pointer -full_pdu, 2,
                            "Invalid character(s) in Time Zone of SMSC Time Stamp: \"%.2s\"", Src_Pointer);
                else
                  Src_Pointer += 2;
              }
            }
          }

          if (!(*err_str))
          {
            if (strlen(Src_Pointer) < 14)
              pdu_error(err_str, 0, Src_Pointer -full_pdu, 14, "While trying to read Discharge Timestamp: %s", err_too_short);
            else
            {
              // get Discharge timestamp
              sprintf(temp,"%c%c-%c%c-%c%c %c%c:%c%c:%c%c",Src_Pointer[1],Src_Pointer[0],Src_Pointer[3],Src_Pointer[2],Src_Pointer[5],Src_Pointer[4],Src_Pointer[7],Src_Pointer[6],Src_Pointer[9],Src_Pointer[8],Src_Pointer[11],Src_Pointer[10]);
              if (!isdigitc(temp[0]) || !isdigitc(temp[1]) || !isdigitc(temp[3]) || !isdigitc(temp[4]) || !isdigitc(temp[6]) || !isdigitc(temp[7]) || 
                  !isdigitc(temp[9]) || !isdigitc(temp[10]) || !isdigitc(temp[12]) || !isdigitc(temp[13]) || !isdigitc(temp[15]) || !isdigitc(temp[16]))
                pdu_error(err_str, 0, Src_Pointer -full_pdu, 12, "Invalid character(s) in Discharge Timestamp: \"%s\"", temp);
              else if (atoi(temp +3) > 12 || atoi(temp +6) > 31 || atoi(temp +9) > 24 || atoi(temp +12) > 59 || atoi(temp +16) > 59)
              {
                // Not a fatal error (?)
                //pdu_error(err_str, 0, Src_Pointer -full_pdu, 12, "Invalid value(s) in Discharge Timestamp: \"%s\"", temp);
                add_warning(warning_headers, "Invalid values(s) in Discharge Timestamp.");
              }

              if (!(*err_str))
              {
                Src_Pointer += 12;
                // Time zone is not used but bytes are checked:
                if (octet2bin_check(Src_Pointer) < 0)
                  pdu_error(err_str, 0, Src_Pointer -full_pdu, 2,
                            "Invalid character(s) in Time Zone of Discharge Time Stamp: \"%.2s\"", Src_Pointer);
                else
                  Src_Pointer += 2;
              }
            }

            if (!(*err_str))
            {
              sprintf(strchr(result, 0), "Discharge_timestamp: %s", temp);
              if (strlen(Src_Pointer) < 2)
                pdu_error(err_str, 0, Src_Pointer -full_pdu, 2, "While trying to read Status octet: %s", err_too_short);
              else
              {
                // get Status
                if ((status = octet2bin_check(Src_Pointer)) < 0)
                  pdu_error(err_str, 0, Src_Pointer -full_pdu, 2, "Invalid Status octet: \"%.2s\"", Src_Pointer);
                else
                {
                  char buffer[128];

                  explain_status(buffer, sizeof(buffer), status);
                  sprintf(strchr(result, 0), "\n%s %i,%s", SR_Status, status, buffer);
                }
              }
            }
          }
        }
      }
    }
  }

  return strlen(result);*/
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

