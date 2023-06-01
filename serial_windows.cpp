#include <iostream>
#include "serial.hpp"

// http://msdn.microsoft.com/en-us/library/ff802693.aspx

Serial::Serial(const char *options, std::function<void(uint8_t)> read)
    :read(read)
{
    if (options == NULL) {
        // o par de portas pre-configurado do com0com é COM3/COM4
        options = "\\\\.\\COM3";
    }

    hComm = CreateFile(options,
                       GENERIC_READ | GENERIC_WRITE, 
                       0,
                       0,
                       OPEN_EXISTING,
                       FILE_FLAG_OVERLAPPED,
                       0);
    if (hComm == INVALID_HANDLE_VALUE) {
        std::cerr << "serial: error opening port: " << GetLastError() << std::endl;
        exit(1);
    }

    DCB dcb = { 0 };
    if (!GetCommState(hComm, &dcb)) {
        std::cerr << "serial: error on GetCommState: " << GetLastError() << std::endl;
        exit(1);
    }

    dcb.BaudRate = 115200;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = NOPARITY;
    dcb.fBinary = TRUE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    dcb.fDsrSensitivity = FALSE;
    dcb.fTXContinueOnXoff = FALSE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;
    dcb.fErrorChar = FALSE;
    dcb.fNull = FALSE;
    dcb.fAbortOnError = FALSE;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;

    if (!SetCommState(hComm, &dcb)) {
        std::cerr << "serial: error on SetCommState: " << GetLastError() << std::endl;
        exit(1);
    }

    COMMTIMEOUTS timeouts;

    timeouts.ReadIntervalTimeout = 1;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;

    if (!SetCommTimeouts(hComm, &timeouts)) {
        std::cerr << "serial: error on SetCommTimeouts: " << GetLastError() << std::endl;
        exit(1);
    }
}

void Serial::write(uint8_t byte)
{
    OVERLAPPED osWrite = {0};
    DWORD dwWritten;
    DWORD dwRes;
    BOOL fRes;

    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (osWrite.hEvent == NULL) {
        std::cerr << "serial write: error creating overlapped event: " << GetLastError() << std::endl;
        return;
    }

    if (!WriteFile(hComm, &byte, 1, &dwWritten, &osWrite)) {
        if (GetLastError() != ERROR_IO_PENDING) {
            std::cerr << "serial write: error on WriteFile: " << GetLastError() << std::endl;
        }
        else {
            dwRes = WaitForSingleObject(osWrite.hEvent, INFINITE);
            switch(dwRes) {
                case WAIT_OBJECT_0:
                    if (!GetOverlappedResult(hComm, &osWrite, &dwWritten, FALSE)) {
                        std::cerr << "serial write: error on GetOverlappedResult: " << GetLastError() << std::endl;
                    }
                    break;
                default:
                    std::cerr << "serial write: error on WaitForSingleObject: " << GetLastError() << std::endl;
                    break;
            }
        }
    }

    CloseHandle(osWrite.hEvent);
}

void Serial::event_loop()
{
    DWORD dwRead;
    OVERLAPPED osReader = {0};
    BOOL fWaitingOnRead = FALSE;

    osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    if (osReader.hEvent == NULL) {
        std::cerr << "serial read: error creating overlapped event: " << GetLastError() << std::endl;
        exit(1);
    }

    while (true) {
        uint8_t buf[1024];

        if (!fWaitingOnRead) {
            if (!ReadFile(hComm, buf, sizeof(buf), &dwRead, &osReader)) {
                if (GetLastError() != ERROR_IO_PENDING) {
                    std::cerr << "serial read: error on ReadFile: " << GetLastError() << std::endl;
                }
                else {
                    fWaitingOnRead = TRUE;
                }
            }
            else {
                for (DWORD i = 0; i < dwRead; i++) {
                    this->read(buf[i]);
                }
            }
        }

        if (fWaitingOnRead) {
            DWORD dwRes = WaitForSingleObject(osReader.hEvent, INFINITE);
            switch(dwRes) {
                case WAIT_OBJECT_0:
                    if (!GetOverlappedResult(hComm, &osReader, &dwRead, FALSE)) {
                        std::cerr << "serial read: error on GetOverlappedResult: " << GetLastError() << std::endl;
                    }
                    else {
                        for (DWORD i = 0; i < dwRead; i++) {
                            this->read(buf[i]);
                        }
                    }
                    fWaitingOnRead = FALSE;
                    break;
                default:
                    std::cerr << "serial read: error on WaitForSingleObject: " << GetLastError() << std::endl;
            }
        }
    }

    CloseHandle(osReader.hEvent);
}
