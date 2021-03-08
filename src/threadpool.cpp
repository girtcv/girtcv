#include "port.hpp"

extern void Append_CRC8_Check_Sum(unsigned char *pchMessage, unsigned int dwLength);
extern unsigned int Verify_CRC8_Check_Sum(unsigned char *pchMessage, unsigned int dwLength);

void ReadSerialPort(void)
{
	ssize_t bRet = false;
	unsigned char* readBufPtr = CSerialPort::GetSerialPort().GetReadBufPtr();
	CSerialPort::GetSerialPort().SetrDataLen(8);

	while (true) {
		bRet = CSerialPort::GetSerialPort().Read();
		if (bRet && readBufPtr[0] == 0xCC && Verify_CRC8_Check_Sum(readBufPtr, 3)) {
			for (size_t i = 0; i < 8; ++i) {
				COUT("%x ", readBufPtr[i]);
			}
			COUT("\n");
		}
	}
}