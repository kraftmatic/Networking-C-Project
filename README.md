# Maria and Nick's Chatroom Application

This repository has both a server and a client for a chatroom application.  The application supports multiple chatroom and a configurable amount of maximum users.

## Installation

The application can be compiled using gcc

```bash
gcc -o server newtcpserver.c -lnsl -lpthread
gcc -o client newtcpclient.c -lnsl -lpthread
```

## Usage

### Server
The server can is configured via the definitions in the c file.  To deploy the server just run the generated script on the server computer.
```bash
./server
```

### Client
The client script takes two arguments: the url of the server and a user name.
```
./client emacs.citadel.edu nick
```

## Notes

Ports for these applications are hard coded as definitions in the file.  

## License

[MIT](https://choosealicense.com/licenses/mit/)
