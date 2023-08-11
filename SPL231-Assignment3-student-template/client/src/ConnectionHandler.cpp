#include "../include/ConnectionHandler.h"

using boost::asio::ip::tcp;

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::thread;



ConnectionHandler::ConnectionHandler(string host, short port) : host_(host), port_(port), io_service_(),
																socket_(io_service_), 
																
																keyBoardListenerShouldTerminate(false), 
																serverListenerShouldTerminate(false),
																username(""),  stompClientProtocol()
																 {}

ConnectionHandler::~ConnectionHandler()
{
	close();
}

bool ConnectionHandler::connect()
{
	std::cout << "Starting connect to "
			  << host_ << ":" << port_ << std::endl;
	try
	{
		tcp::endpoint endpoint(boost::asio::ip::address::from_string(host_), port_); // the server endpoint
		boost::system::error_code error;
		socket_.connect(endpoint, error);
		if (error)
			throw boost::system::system_error(error);
	}
	catch (std::exception &e)
	{
		std::cerr << "Connection failed (Error: " << e.what() << ')' << std::endl;
		return false;
	}
	return true;
}

bool ConnectionHandler::getBytes(char bytes[], unsigned int bytesToRead)
{
	size_t tmp = 0;
	boost::system::error_code error;
	try
	{
		// cout << "boolean of error is: " << error << endl;
		// cout << "number of bytes to read: " << bytesToRead << endl;
		while (!error && bytesToRead > tmp)
		{
			tmp += socket_.read_some(boost::asio::buffer(bytes + tmp, bytesToRead - tmp), error);
		}
		if (error)
			throw boost::system::system_error(error);
	}
	catch (std::exception &e)
	{
		// std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
		return false;
	}
	return true;
}

bool ConnectionHandler::sendBytes(const char bytes[], int bytesToWrite)
{
	int tmp = 0;
	boost::system::error_code error;
	try
	{
		while (!error && bytesToWrite > tmp)
		{
			tmp += socket_.write_some(boost::asio::buffer(bytes + tmp, bytesToWrite - tmp), error);
		}
		if (error)
			throw boost::system::system_error(error);
	}
	catch (std::exception &e)
	{
		std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
		return false;
	}
	return true;
}

bool ConnectionHandler::getLine(std::string &line)
{
	return getFrameAscii(line, '\n');
}

bool ConnectionHandler::sendLine(std::string &line)
{
	return sendFrameAscii(line, '\n');
}

bool ConnectionHandler::getFrameAscii(std::string &frame, char delimiter)
{
	char ch;
	// Stop when we encounter the null character.
	// Notice that the null character is not appended to the frame string.
	try
	{
		do
		{
			// cout << "IM IN GET FRAME ASCII" << endl;
			if (!getBytes(&ch, 1))
			{
				return false;
			}
			if (ch != '\0')
				frame.append(1, ch);
		} while (delimiter != ch);
	}
	catch (std::exception &e)
	{
		std::cerr << "recv failed2 (Error: " << e.what() << ')' << std::endl;
		return false;
	}
	return true;
}

bool ConnectionHandler::sendFrameAscii(const std::string &frame, char delimiter)
{
	bool result = sendBytes(frame.c_str(), frame.length());
	if (!result)
		return false;
	return sendBytes(&delimiter, 1);
}

// Close down the connection properly.
void ConnectionHandler::close()
{
	try
	{
		socket_.close();
	}
	catch (...)
	{
		std::cout << "closing failed: connection already closed" << std::endl;
	}
}

void ConnectionHandler::runKeyBoardListener()
{

	// while (!interupted && !clientShouldTerminate)
	while (!IsKeyBoardListenerShouldTerminate())
	{
		// get a string in a line from the keyboard.
		// cout <<"I started the loop of the THREAD" << endl;
		const short bufsize = 1024;
		char buf[bufsize];
		std::cin.getline(buf, bufsize);
		std::string line(buf);

		// queue<string> framesAsStringsQ = stompClientProtocol.processFromKB(line);
		vector<string> framesAsStringsQ = stompClientProtocol.processFromKB(line);
		// cout << "THE -TO SEND- FRAMES Q SIZE IS: " << framesAsStringsQ.size() << endl;

		int i = 0;
		while (i < framesAsStringsQ.size() & !IsKeyBoardListenerShouldTerminate())
		{

			// cout << "AM I HERE AND ITS NOT EMPTY??" << endl;
			// string frameToSendAsString = framesAsStringsQ.front(); // get the element
			// framesAsStringsQ.pop();	// removes this element
			string frameToSendAsString = framesAsStringsQ.at(i);

			//check if the command is DISCONNECT if so - turn off the keyboard thread.
			int indexOfEndOfCommand = frameToSendAsString.find_first_of('\n');
			string command = frameToSendAsString.substr(0, indexOfEndOfCommand);
			sendFrameAscii(frameToSendAsString, '\0');
			

			std::this_thread::sleep_for(std::chrono::milliseconds(300));

			// cout << "FRAME WAS SENT" << endl;

			if (command == "DISCONNECT"){
				keyBoardListenerShouldTerminate = true;
				// cout << "KEY BOARD SHOULD TERMINATE " << endl;
			}

			i++;
		}
		// framesAsStringsQ.clear();
		// cout << "I FINISHED THE LOOP OF THE THREAD" << endl;
	}

	// calls processFromKB() with the line
}

void ConnectionHandler::runServerListener()
{
	// while (!interupted && !clientShouldTerminate)
	while (!IsServerListenerShouldTerminate())
	{
		// call getFrameAscii with empty string
		// call stringToFrame with the string
		string emptyString;
		getFrameAscii(emptyString, '\0'); // get the string from the server
		
		if (emptyString.size() != 0){
		// cout << "Got a string from the server (IM AT CONNECTION HANDLER): " << emptyString << "of size: " << emptyString.size() << "\n" << endl;

		Frame frameFromServer = stompClientProtocol.stringToFrameFromServer(emptyString);
		string commandFromFrame = frameFromServer.getCommand();
		stompClientProtocol.processFromServer(frameFromServer);
		
		if (stompClientProtocol.getServerShouldTerminate()){
			// cout << "server should terminate" << endl;
			serverListenerShouldTerminate = true;
		}
		if (commandFromFrame == "ERROR"){ // error -> disconnect the client.
			// cout << "AN error - AT CONNECTION HANDLER" << endl;
			string messageFromFrame = frameFromServer.getHeaders()["message"];
			if (messageFromFrame != "User already logged in, log out before trying again."){
				cout << "You got an error. you get dissconnected" << endl;
				serverListenerShouldTerminate = true;
				keyBoardListenerShouldTerminate = true;}
		}
		}
	}
}

bool ConnectionHandler::IsKeyBoardListenerShouldTerminate()
{
	return keyBoardListenerShouldTerminate;
}

bool ConnectionHandler::IsServerListenerShouldTerminate()
{
	return serverListenerShouldTerminate;
}

void ConnectionHandler::setUserName(string user)
{
	username = user;
	stompClientProtocol.setUserName(user);
}

// void ConnectionHandler::setKeyBoardThread(thread& t){
// 	keyBoardListener = t;
// }

