# epic-wall
C++ implementation of a "wall" server where users can connect and *tag* the wall with their own messages. 

Used as an exercise to teach myself socket programming in C++.

## Features
**Posting**: Clients connected the server can type in the `post` command along with their name to post a message on the wall.
**Clearing**: Clients can clear wall messages with `clear`.
**Quitting**: Users can exit the server program and preserve the current wall messages with `quit`.
**Killing**: Users can kill the server-side process and disconnect with the `kill` command. This command clears all messages.

## Instructions
Compile the server-side program with `make`, then execute it on the server with `./wallserver <queue size> <port (5514 by default)>`.
Then, using netcat a client machine, connect to it with `nc <server ip> <server port (5514 by default)>`.
