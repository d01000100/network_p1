# network_p1
Chat app with channels and deserialization

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