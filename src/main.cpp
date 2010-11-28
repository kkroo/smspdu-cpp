#include <iostream>
#include "pdu.h"
#include <iconv.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

using namespace std;


size_t convert(const char *to_enc, const char *from_enc, char *in_str, 
		char *out_str, size_t out_len)
{
    if(!to_enc || !in_str || !out_str)
        return -1;

	size_t bytes_in, bytes_out, bytes_converted;
	bytes_in = strlen(in_str);
	bytes_out = out_len;
	
	iconv_t cd = iconv_open(to_enc, from_enc);
	if(cd == (iconv_t)(-1))
	{
		printf("iconv_open(...) failed. Errno %d\n", errno);
		return -1;
	}

	bytes_converted = iconv(cd, &in_str, &bytes_in, &out_str, &bytes_out);
	if(bytes_converted < 0)
		printf("iconv(...) failed. Errno %d\n", errno);

	if(iconv_close(cd) != 0)
		printf("iconv_close(...) failed. Errno %d\n", errno);
	
	if(bytes_converted >= 0 && bytes_out > 0)
		out_str[bytes_converted] = '\0';

	return bytes_converted;
}

int main()
{
    //PDU pdu("0791839001446010040C9183904517829100000111726115318005C8329BFD06"); // Hello
    PDU pdu("0791839001446010040C918390314598910008011182417025800C041F04400438043204350442"); // Привет(ru_RU.UTF-8)
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
        
        char in_str[512], out_str[512];
        strcpy(in_str, pdu.getText().c_str());
        if (convert("utf8", "UTF16BE", in_str, out_str, 512) >= 0)
            cout << "PDU Message: " << out_str << endl;
        else
            cout << "PDU Message: " << pdu.getText() << endl;
        
        cout << "PDU UDH Type: " << pdu.getUDHType() << endl;
        cout << "PDU UDH Data: " << pdu.getUDHData() << endl;
    }

    return 0;
}

