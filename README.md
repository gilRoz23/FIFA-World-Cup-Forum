# World Cup Forum

This repository contains the completed code for the World CupForum.

## Files

- `server/`: Contains all the server-side code files (.java).
- `client/`: Contains all the client-side code files (.cpp).
- `examples/`: Contains example code for different protocols and clients.
- `Makefile`: Compiles the server and client code into executable JAR and binary files.

## How to Compile

### Server

Navigate to the `server/` directory and run the following command to compile the server code:

```bash
make server
```
# Client

Navigate to the `client/` directory and run the following command to compile the client code:

```bash
make client
```
# Running the Program
Start the Server
Run the following command to start the server with the desired server model (TPC or Reactor) and port number:

```bash
java -jar server.jar --model=TPC --port=8080
```
# Start the Client
Run the following command to start the client and interact with the server:
```bash
./client
```
