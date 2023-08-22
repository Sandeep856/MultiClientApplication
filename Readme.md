Readme File
Sandeep Rout
2020A7PS1711G Computer Networks Lab 7
In this assignment, two clients communicate with each other via the server. Any client can send a message to the other client at any time of their choice.  When one client sends a message to the server, the server creates a text file in the server and stores the data (message) sent by the client. The file name is clientid1_clientid2.txt. The server then sends the message to the other client that reads the message and displays its content on its terminal.

The code contains the program for both the server and client and the arguments will be the server's IP address, the server's port number, and the mode of running (c for client, s for server).

To run the program first type in the server’s IP address and port number and the mode of running as s or c.

The client-side program will ask for the username of the client, type the username of the client and press enter the server will receive the username of the current client and store it for future use. 

After connecting at least two clients they can now communicate with each other, to send messages from one client to the other type in “MESG       Client Username to whom to send        and the message to be sent”, and press enter. The message will be displayed on the terminal of the target client and an entry will be created in the file named “clientid1_clientid2.txt”.

