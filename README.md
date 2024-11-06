# SerialSend

SerialSend is a little command line application originally created by Ted Burke (https://batchloaf.com/) to send text strings via a serial port. It is mainly used to send information to microcontroller circuits via a USB-to-serial converter, so it’s designed to work well in that context.

SerialSend lets you:

* Send an arbitrary text string to a device via serial port using one simple command
* Send text from simple console applications to hardware devices via serial port using the “system” function
* Specify baud rate
* Specify serial port number
* Specify dtr
* Automatically find and use the highest available serial port number (useful for USB-to-serial converters, Arduinos, etc.)

## Usage

Note: If the text to be transmitted contains any space characters, it should be enclosed in inverted commas.

The following command sends the characters “abc 123” via the highest available serial port at the default baud rate (9600 baud).
```bash
SerialSend.exe "abc 123"
```
The following command sends the characters “Hello world!” via the highest available serial port at 115200 baud.
```bash
SerialSend.exe /baudrate 115200 "Hello world!"
```

The following command sends the characters “S120 E360” via COM10 at the default baud rate (9600 baud). If COM10 is not available, the next highest serial port that is available is used instead.
```bash
SerialSend.exe /devnum 10 "S120 E360"
```

Arbitrary bytes, including non-printable characters can be included in the string as hex values using the “/hex” command line option and the “\x” escape sequence in the specified text. For example, the following command sends the string “abc” followed by a line feed character (hex value 0x0A) – i.e. 4 bytes in total.
```bash
SerialSend.exe /hex "abc\x0A"
```

When the “/hex” commmand line option is specified, the escape sequences “\n” and “\r” may be used to insert line feed and carriage return characters respectively. For example, the following command sends the string “Hello” followed by a carriage return and a line feed (7 bytes in total).
```bash
SerialSend.exe /hex "Hello\r\n"
```

The “/closedelay” command line option allows a delay (in milliseconds) to be carried out after the specified text is transmitted, but before the serial port is closed. This seems to be necessary when sending data to certain devices in order to give them time to respond. For example, the following command transmits the characters “ABCD“ and a carriage return to COM5, then delays for 500 ms before closing the COM port.
```bash
SerialSend.exe /devnum 5 /closedelay 500 "ABCD\r"
```

The “/noscan” command line option prevents SerialSend from trying additional devices if the first device cannot be opened. This would normally be used together with the “/devnum” option. For example, the following command sends the characters “hello“ to COM5 if available, but will not try any other ports if COM5 is not available.
```bash
SerialSend.exe /devnum 5 /noscan "hello"
```

To select even parity, include the “/evenparity” command line option.
```bash
SerialSend.exe /devnum 3 /evenparity /baudrate 9600 "hello"
```

To select odd parity, include the “/oddparity” command line option.
```bash
SerialSend.exe /devnum 3 /oddparity /baudrate 9600 "hello"
```

By default DTR ist set to ON, to disable it, include “/dtr 0” command line option.
```bash
SerialSend.exe /devnum 3 /oddparity /baudrate 9600 /dtr 0 "hello"
```

The “/quiet” command line option suppresses all printed output to the console. For example, the following command sends the characters “hello“ to COM10 at 9600 baud, but without printing any messages in the console.
```bash
SerialSend.exe /quiet /devnum 10 /baudrate 9600 "hello"
```
