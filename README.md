# network_p1
Chat app with channels and deserialization

- José Daniel Casado Aguilar
- David Zúñiga Navarro

## Build instructions

1. Open the file `chatroom\chatroom.sln` with Visual Studio 2019.
2. Select any Configuration (Debug or Release) and any Platform (x64 or x86) and **Build Solution** (Ctrl + Shift + B).
3. Visual Studio will generate executable files on the appropiate folder at the solution level (Debug, Release x64\Debug, x64\Release).

## Run instructions

1. Navigate to the folder where the executable files were built.
2. Run the `Server.exe` file. If everything runs correctly, a console window will show saying "Server ready!".
3. For each client needed, run `Client2.exe`.

### Use instructions (Client)

1. In the Client console window, first type a username (the name of this client to the server and other clients) and hit enter.
2. A console will appear with a message log from rooms you are a member of and a prompt to type messages.
3. To join a room (and create one if it doesn't exist) type ".join room_name" and press Enter. 
4. To leave a room you're in type ".leave room_name" and press Enter.
5. To send a message to a room you're in type ".send room_name message" and press Enter.
6. To close the client hit Escape.
7. To close the server hit Escape.

## Message Protocol

**Messages types** (enum)

- **LOGIN**  
  Message send from the client to the server to register itself. Contains the name of the client in which other clients will see their messages.
- **JOIN**  
  Message from the client to the server to join a room in order to send and recieve messages in the room.
- **LEAVE**  
  Message from the client to the server to leave a room and stop getting messages from it.
- **SEND**  
  Message from the client to the server to broadcast to other clients. Contains what room to send to message to and the content of the message.
- **RECIEVE**  
  Message from the server to a client to notify them about a new message in a room they're subscribed to. Contains the name of the sender, the name of the room and the contents of the message.

### Header

All messages begin with this 2 integers:

```
      int             int
[packet_length] [message_type]
```
### Login

```
   *       int        string
[Header] [length] [client_name]
````
### Join Room
```
   ?       int      string
[Header] [length] [room_name]
```
### Leave Room
```
   ?       int      string
[Header] [length] [room_name]
```
### Send Message

```
   *       int      string      int     string
[Header] [length] [room_name] [length] [message]
````

### Recieve Message

```
   ?        int       string       int     string      int    string
[Header] [length] [sender_name] [length] [room_name] [length] [message]
```