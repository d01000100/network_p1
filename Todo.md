## ProtoBuff and protocol

1. Design protocol and message structures
2. Compile messages and serialize/deserialize functions ??
   - Do we seriallize to string or to bytes?
   - Can we read one byte/int at a time from the serialized proto buff??

## Database and C++ integration

1. Implement tables
2. Implement writing and reading from such tables
   1. Create user.
   2. Log in user.

## Hashing and verification of passwords

1. Integrate OpenSSL (or another SHA256 hashing) into the project
2. Implement functions to:
   1. Create a salt.
   2. Hash a password combining it with a salt.
   3. Check if a plain password and a salt matches a hash.

## Chat Server and Auth Server communication

1. Create client.
   Possible responses:
   - OK
   - User already created (same username/email)
   - Database/Communication error
2. Login Client:
   Possible responses:
   - OK
   - User non existent / Wrong password
   - Database/Communication error

## New functions in the client and server communication

1. Sign up
2. Login
3. Logout

### BONUS

- The server doesn't reciveve any other messages form the user until it has logged in