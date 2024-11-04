//
// SerialSend.c - This program sends text via serial port
// Written by Ted Burke - last updated 10-Jan-2022
//
// The text to send is specified as command line arguments.
// By default, the highest available serial port is used.
// The default baud rate is 38400 baud.
//
// To compile with MinGW:
//
//      gcc -o SerialSend.exe SerialSend.c
//
// To compile with cl, the Microsoft compiler:
//
//      cl SerialSend.c
//
// To run (this example sends the characters "S365 E120"):
//
//      SerialSend.exe "S356 E120"
//
    
#include <windows.h>
#include <stdio.h>
    
int main(int argc, char *argv[])
{
    // Declare variables and structures
    int m, n;
    unsigned char buffer[MAX_PATH];
    unsigned char text_to_send[MAX_PATH];
    unsigned char digits[MAX_PATH];
    int baudrate = 9600;
    int dev_num = 50;
	int dtr = 1;
    int parse_hex_bytes = 0;
    int close_delay = 0;
    char dev_name[MAX_PATH];
    int quiet = 0;
    int no_scan = 0;
    int even_parity = 0, odd_parity = 0;
    HANDLE hSerial;
    DCB dcbSerialParams = {0};
    COMMTIMEOUTS timeouts = {0};
    
    // Check if one of the command line args is "/quiet"
    int argn;
    for (argn = 1 ; argn < argc ; ++argn)
    {
        if (strcmp(argv[argn], "/quiet") == 0) quiet = 1;
    }
       
    // Print welcome message
    if (!quiet) fprintf(stderr, "SerialSend (last updated 10-Jan-2022)\n");
    if (!quiet) fprintf(stderr, "See http://batchloaf.com for more information\n");
    
    // Parse the rest of the command line arguments
    strcpy(buffer, "");
    argn = 1;
    while(argn < argc)
    {
        if (strcmp(argv[argn], "/baudrate") == 0)
        {
            // Parse baud rate
            if (++argn < argc && ((baudrate = atoi(argv[argn])) > 0))
            {
                if (!quiet) fprintf(stderr, "%d baud specified\n", baudrate);
            }
            else
            {
                if (!quiet) fprintf(stderr, "Baud rate error\n");
                return 1;
            }
        }
        else if (strcmp(argv[argn], "/devnum") == 0)
        {
            // Parse device number. SerialSend actually just
            // begins searching at this number and continues
            // working down to zero.
            if (++argn < argc)
            {
                dev_num = atoi(argv[argn]);
                if (!quiet) fprintf(stderr, "Device number %d specified\n", dev_num);
            }
            else
            {
                if (!quiet) fprintf(stderr, "Device number error\n");
                return 1;
            }
        }
        else if (strcmp(argv[argn], "/closedelay") == 0)
        {
            // Parse close delay duration. After transmitting
            // the specified text, SerialSend will delay by
            // this number of milliseconds before closing the
            // COM port. Some devices seem to require this.
            if (++argn < argc)
            {
                close_delay = atoi(argv[argn]);
                if (!quiet) fprintf(stderr, "Delay of %d ms specified before closing COM port\n", close_delay);
            }
            else
            {
                if (!quiet) fprintf(stderr, "Close delay error\n");
                return 1;
            }
        }
        else if (strcmp(argv[argn], "/noscan") == 0)
        {
            // Set the no_scan flag, so that SerialSend will exit if
            // the specified device is not available rather than
            // scanning other device numbers.
            no_scan = 1;
            if (!quiet) fprintf(stderr, "no_scan selected, so only one device will be tried\n");
        }
        else if (strcmp(argv[argn], "/evenparity") == 0)
        {
            // Set the even_parity flag.
            even_parity = 1;
            if (!quiet) fprintf(stderr, "Even parity selected\n");
        }
        else if (strcmp(argv[argn], "/oddparity") == 0)
        {
            // Set the odd_parity flag.
            odd_parity = 1;
            if (!quiet) fprintf(stderr, "Odd parity selected\n");
        }
		else if (strcmp(argv[argn], "/dtr") == 0)
        {
            if (++argn < argc)
            {
                dtr = atoi(argv[argn]);
                if (!quiet) fprintf(stderr, "DTR %d specified\n", dtr);
            }
            else
            {
                if (!quiet) fprintf(stderr, "DTR error\n");
                return 1;
            }
        }
        else if (strcmp(argv[argn], "/hex") == 0)
        {
            // Parse flag for hex byte parsing.
            // If this flag is set, then arbitrary byte values can be
            // included in the string to send using '\x' notation.
            // For example, the command "SerialSend /hex Hello\x0D"
            // sends six bytes in total, the last being the carriage
            // return character, '\r' which has hex value 0x0D.
            parse_hex_bytes = 1;
        }
        else
        {
            // This command line argument is the text to send
            strcpy(buffer, argv[argn]);
        }
    
        // Next command line argument
        argn++;
    }
    
    // Check that some text to send was provided
    if (strlen(buffer) == 0)
    {
        if (!quiet) fprintf(stderr, "Usage:\n\n\tSerialSend [/quiet] [/noscan] [/baudrate BAUDRATE] ");
        if (!quiet) fprintf(stderr, "[/devnum DEVICE_NUMBER] [/hex] \"TEXT_TO_SEND\"\n");
        return 1;
    }
    
    // If hex parsing is enabled, modify text to send
    n = 0; m = 0;
    while(n < strlen(buffer))
    {
        if (parse_hex_bytes && buffer[n] == '\\')
        {
            n++;
            if (buffer[n] == '\\') text_to_send[m] = '\\';
            else if (buffer[n] == 'n') text_to_send[m] = '\n';
            else if (buffer[n] == 'r') text_to_send[m] = '\r';
            else if (buffer[n] == 'x')
            {
                digits[0] = buffer[++n];
                digits[1] = buffer[++n];
                digits[2] = '\0';
                text_to_send[m] = strtol(digits, NULL, 16);
            }
        }
        else
        {
            text_to_send[m] = buffer[n];
        }
    
        m++; n++;
    }
    text_to_send[m] = '\0'; // Null character to terminate string
    
    // Open the highest available serial port number
    if (!quiet) fprintf(stderr, "Searching serial ports...\n");
    while(dev_num >= 0)
    {
        if (!quiet) fprintf(stderr, "\r                        ");
        if (!quiet) fprintf(stderr, "\rTrying COM%d...", dev_num);
        sprintf(dev_name, "\\\\.\\COM%d", dev_num);
        hSerial = CreateFile(
            dev_name, GENERIC_READ|GENERIC_WRITE, 0, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        if (hSerial == INVALID_HANDLE_VALUE)
        {
            // If no_scan option was specified, don't try other devices.
            // Otherwise, decrement device number.
            if (no_scan) dev_num = -1;
            else dev_num--;
        }
        else break;
    }
    
    if (dev_num < 0)
    {
        if (!quiet) fprintf(stderr, "No serial port available\n");
        return 1;
    }
    
    if (!quiet) fprintf(stderr, "OK\n");
    
    // Set device parameters (38400 baud, 1 start bit,
    // 1 stop bit, no parity)
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (GetCommState(hSerial, &dcbSerialParams) == 0)
    {
        if (!quiet) fprintf(stderr, "Error getting device state\n");
        CloseHandle(hSerial);
        return 1;
    }
    //dcbSerialParams.BaudRate = CBR_38400;
    dcbSerialParams.BaudRate = baudrate;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = even_parity ? EVENPARITY : odd_parity ? ODDPARITY : NOPARITY;
	dcbSerialParams.fDtrControl = dtr ? DTR_CONTROL_ENABLE : DTR_CONTROL_DISABLE;
    if(SetCommState(hSerial, &dcbSerialParams) == 0)
    {
        if (!quiet) fprintf(stderr, "Error setting device parameters\n");
        CloseHandle(hSerial);
        return 1;
    }
    
    // Set COM port timeout settings
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if(SetCommTimeouts(hSerial, &timeouts) == 0)
    {
        if (!quiet) fprintf(stderr, "Error setting timeouts\n");
        CloseHandle(hSerial);
        return 1;
    }
    
    // Send specified text
    DWORD bytes_written, total_bytes_written = 0;
    if (!quiet) fprintf(stderr, "Sending text... ");
    while(total_bytes_written < m)
    {
        if(!WriteFile(hSerial, text_to_send + total_bytes_written,
            m - total_bytes_written, &bytes_written, NULL))
        {
            if (!quiet) fprintf(stderr, "Error writing text to %s\n", dev_name);
            CloseHandle(hSerial);
            return 1;
        }
    
        total_bytes_written += bytes_written;
    }
    if (!quiet) fprintf(stderr, "\n%d bytes written to %s\n", total_bytes_written, dev_name);
    
    // Flush transmit buffer before closing serial port
    FlushFileBuffers(hSerial);
    if (close_delay > 0)
    {
        if (!quiet) fprintf(stderr, "Delaying for %d ms before closing COM port... ", close_delay);
        Sleep(close_delay);
        if (!quiet) fprintf(stderr, "OK\n");
    }
    
    // Close serial port
    if (!quiet) fprintf(stderr, "Closing serial port...");
    if (CloseHandle(hSerial) == 0)
    {
        if (!quiet) fprintf(stderr, "Error\n", dev_name);
        return 1;
    }
    if (!quiet) fprintf(stderr, "OK\n");
    
    // exit normally
    return 0;
}
