#include <errhandlingapi.h>
#include <fileapi.h>
#include <handleapi.h>
#include <ioapiset.h>
#include <iostream>
#include <winerror.h>
#include "serial.hpp"

// http://msdn.microsoft.com/en-us/library/ff802693.aspx

Serial::Serial(const char *options, std::function<void(uint8_t)> read)
    :read(read)
{
    if (options == NULL) {
        // o par de portas pre-configurado do com0com é COM3/COM4
        options = "COM3";
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

    DCB dcbSerialParams = { 0 };
    if (!GetCommState(hComm, &dcbSerialParams)) {
        std::cerr << "serial: error on GetCommState: " << GetLastError() << std::endl;
        exit(1);
    }

    dcbSerialParams.BaudRate = 115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    dcbSerialParams.fDtrControl = DTR_CONTROL_DISABLE;

    if (!SetCommState(hComm, &dcbSerialParams)) {
        std::cerr << "serial: error on SetCommState: " << GetLastError() << std::endl;
        exit(1);
    }

    PurgeComm(hComm, PURGE_RXCLEAR | PURGE_TXCLEAR);
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
