C++ class for encoding/decoding SMS in PDU mode.

Based on smstools3 (http://smstools3.kekekasvi.com/)

Used in project http://code.google.com/p/yate-datacard/

**Works:**
  * reading GSM encoded Deliver Type Messages
  * reading UCS2 encoded Deliver Type Messages (with `iconv`)
  * writing PDU in GSM/UCS2 encodings
  * reading Status Report Messages
  * sending flash sms
**Not tested:**
  * reading binary data from Deliver Type Messages
