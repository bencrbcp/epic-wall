#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
using namespace std;

/*
 * Two functions, sendstring() and recvstring(), were made by me to facilitate message
 * passing between the client and server using std::strings. Char vectors are used to
 * store the physical bytes.
 */
void sendstring(string &stringbuf, vector<char> &buf, int &client_fd, int &server_fd)
{
    std::copy(stringbuf.begin(), stringbuf.end(), buf.begin());
    send(server_fd, &buf[0], (size_t)(stringbuf.length()), 0);
    buf = vector<char>(1024);
}

void recvstring(string &stringbuf, vector<char> &buf, int &client_fd, int &server_fd)
{
    stringbuf = "";
    int bytesReceived = recv(server_fd, &buf[0], buf.size(), 0);
    if (bytesReceived == -1) {
        close(server_fd);
        close(client_fd);
        throw std::runtime_error("Error receiving bytes");
    }
    else {
        stringbuf.append( &buf[0], (char *)(&buf[0]) + (size_t)(bytesReceived - 1) );
    }

}

int main(int argc, char **argv)
{
    /* declaring socket variables */
    int queue_size = 20;
    int port = 5514;
    struct sockaddr_in server_address;
    socklen_t size = sizeof(sockaddr_in);
    vector<char> buf(1024);
    string stringbuf;
    int opt = 1;
    
    /* parsing server launch options */
    if (argc > 1) {
        queue_size = atoi(argv[1]);
        if (argc == 3)
            port = atoi(argv[2]);
        else
            throw std::runtime_error("Error: please provide at most 2 command-line arguments");
    }
    
    /* file descriptors for the client and server sockets */
    int server_fd;
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    if (client_fd < 0)
        throw std::runtime_error("Socket creation failed.");

    /* setting up server to listen on local network */
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htons(INADDR_ANY);
    server_address.sin_port = htons(port);
    
    if (bind(client_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        close(server_fd);
        close(client_fd);
        throw std::runtime_error("Error binding socket");
    }

    size = sizeof(server_address);

    listen(client_fd, 1);
    server_fd = accept(client_fd, (struct sockaddr *)&server_address, &size);
    
    if (server_fd < 0) {
        close(server_fd);
        close(client_fd);
        throw std::runtime_error("Error accepting client");
    }





    deque<string> tags; // user tags are stored here in '<NAME>: <MESSAGE>' format
    int server_closed = 1; // counter-intuitively, 1 means this file descriptor is closed, and 0 means it is open and available
    /*
     * MAIN SERVER LOOP BEGIN
     */
    while(1)
    {
        if ( server_closed == 0) {
            /* cout <<"client_fd =" << endl; */
            /* client_fd = socket(AF_INET, SOCK_STREAM, 0); */
            /* setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)); */
            /* cout <<"listen()" << endl; */
            /* listen(client_fd, 1); */
            /* cout <<"accept()" << endl; */
            server_fd = accept(client_fd, (struct sockaddr *)&server_address, &size);
            server_closed = 1;
        }
        stringbuf = "Wall contents\n-------------\n";
        sendstring(stringbuf, buf, client_fd, server_fd);

        if (tags.empty()) {
            stringbuf = "[NO MESSAGES - WALL EMPTY]\n\n";
            sendstring(stringbuf, buf, client_fd, server_fd);
        }
        for (int i=0; i < tags.size(); i++) {
            stringbuf = tags.at(i);
            if (i == tags.size() - 1)
                stringbuf += "\n";

            sendstring(stringbuf, buf, client_fd, server_fd);
        }

        stringbuf = "Enter command: ";
        sendstring(stringbuf, buf, client_fd, server_fd);
        recvstring(stringbuf, buf, client_fd, server_fd);

        /* CODE TO TEST STRING CONVERSION */
        /* cout << "bytes = " << bytesReceived << endl << "command = " << command << endl; */
        /* cout << "comparing input to \"post\": " << command.compare("post") << endl; */

        /* --------------------- */
        /* BEGIN COMMAND OPTIONS */
        /* --------------------- */
        
        /* CLEAR COMMAND */
        if (stringbuf == "clear") {
            tags.clear();
            stringbuf = "Wall cleared.\n\n";
            sendstring(stringbuf, buf, client_fd, server_fd);
        }

        /* POST COMMAND */
        else if (stringbuf == "post") {
            stringbuf = "Enter name: ";
            sendstring(stringbuf, buf, client_fd, server_fd);
            recvstring(stringbuf, buf, client_fd, server_fd); 

            string newtag = stringbuf + ": ";
            int max_msglen = 80 - newtag.length();

            if (max_msglen < 1) {
                stringbuf = "Error: name is too long!\n\n";
                sendstring(stringbuf, buf, client_fd, server_fd);
                continue;
            }

            stringbuf = "Post [Max length " + std::to_string(max_msglen) + "]: ";
            sendstring(stringbuf, buf, client_fd, server_fd);
            recvstring(stringbuf, buf, client_fd, server_fd); 
            
            if (stringbuf.length() > max_msglen) {
                stringbuf = "Error: message is too long!\n\n";
                sendstring(stringbuf, buf, client_fd, server_fd);
                continue;
            }
            else {
                newtag += stringbuf +"\n";
                if (tags.size() >= queue_size)
                    tags.pop_front();
                tags.push_back(newtag);
                stringbuf = "Successfully tagged the wall\n\n";
                sendstring(stringbuf, buf, client_fd, server_fd);
            }


        }
        
        /* KILL COMMAND */
        else if (stringbuf == "kill") {
            stringbuf = "Closing socket and terminating server. Bye!\n";
            sendstring(stringbuf, buf, client_fd, server_fd);
            int close_server = close(server_fd);
            int close_client = close(client_fd);
            exit(0);
        }

        /* QUIT COMMAND */
        else if (stringbuf == "quit") {
            stringbuf = "Come back soon. Bye!\n";
            sendstring(stringbuf, buf, client_fd, server_fd);
            server_closed = close(server_fd); // returns 0 if closed successfully
            if (server_closed == -1) 
                cout << "Error closing client on quit" << endl;
        }
    }
    return 0;
}
