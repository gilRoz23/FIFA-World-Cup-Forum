package bgu.spl.net.impl.stomp;
import bgu.spl.net.api.StompMessagingProtocol;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;

public class StompProtocol implements StompMessagingProtocol<Frame> {

    private boolean shouldTerminate = false;
    private int connectionId;
    ConnectionsImpl<Frame> connections;

    
    public void start(int connectionId, ConnectionsImpl<Frame> connections){
        this.connectionId = connectionId;
        this.connections = connections;
    /**
	 * Used to initiate the current client protocol with it's personal connection ID and the connections implementation
	**/
    }
    
    public void process(Frame msg) {
        String command = msg.getCommand();
        
        System.out.println("THE COMMAND THE SERVER GOT IS " + command);

        shouldTerminate = "DISCONNECT".equals(command);
        if(command.equals("CONNECT"))
            connectProcess(msg);
        else{ //not CONNECT request
            if(connections.getUsernameByConnectionId(connectionId) == null){ //user isnt loged-in
                createRegularErrorFrameAndSendAndDisconnect(msg, "first you must log-in"); //send error and disconnect
            }
            else{ //user is logged-in
                if(command.equals("SEND"))
                    sendProcess(msg);
                else if(command.equals("SUBSCRIBE"))
                    subscribeProcess(msg);
                else if(command.equals("UNSUBSCRIBE"))
                    unsubscribeProcess(msg);
                else if(command.equals("DISCONNECT"))
                    disconnectProcess(msg); 
                else{
                    createRegularErrorFrameAndSendAndDisconnect(msg, "invalid Command");
                }
            }
        }                       
    }
    
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    //private processes
    
    private void connectProcess(Frame message){
        Set<String> expectedHeaders = new HashSet<String>(Arrays.asList("accept-version", "host", "login", "passcode"));

        if(!messageValidityCheck(message, expectedHeaders)){ //malformed message
            createMalFormedErrorFrameAndSendAndDisconnect(message, expectedHeaders); //send error and disconnect
        }
        else if (!message.getHeaders().get("accept-version").equals("1.2") ){
            createRegularErrorFrameAndSendAndDisconnect(message, "version is wrong");
        }
        else if (!message.getHeaders().get("host").equals("stomp.cs.bgu.ac.il") ){
            createRegularErrorFrameAndSendAndDisconnect(message, "host is wrong");
        }

        else{ //Frame is valid
            String userName = message.getHeaders().get("login");
            String loginPasscode = message.getHeaders().get("passcode");
            User user = connections.getUserByUsername(userName);
            if(user!=null){ //user exists in data-base
                if(!user.isConnected()){ //user is not already connected
                    if(user.isCorrectPasscode(loginPasscode)){ //passcode is correct
                        connections.connect(connectionId, userName); //connect it
                        createConnectedFrameAndSend(); //generate CONNECTED frame and send it to myself through Connections
                    }
                    else{ //password is wrong
                        createRegularErrorFrameAndSendAndDisconnect(message, "Wrong password"); //send error and disconnect
                    }
                }
                else{ //user already connected
                    createRegularErrorFrame(message, "User already logged in, log out before trying again.");
                }
            }
            else{ //there is no username like this in data-base: create new one and connect it
                user = new User(loginPasscode);
                connections.registerUser(userName, user);
                connections.connect(connectionId, userName);
                createConnectedFrameAndSend(); //generate CONNECTED frame and send it to myself through Connections
            }
        }
    }
    
    
    
    private void sendProcess(Frame message){
        Set<String> expectedHeaders = new HashSet<String>(Arrays.asList("destination"));
        if(!messageValidityCheck(message, expectedHeaders)){ //message invalid
            createMalFormedErrorFrameAndSendAndDisconnect(message, expectedHeaders); //send error and disconnect
        }
        else{ //message is valid
            String channel = message.getHeaders().get("destination");
            User user = connections.getUserByConnectionId(connectionId);
            String subscriptionId = user.getSubscriptionIdByChannel(channel);
            if(subscriptionId == null){ //client isn't subscribe to this channel
                createRegularErrorFrameAndSendAndDisconnect(message, "User isnt subscribed to this channel"); //send error and disconnect
            }
            else{ //client is subscribed to the channel
                String bodyMessage = message.getBody();
                createMessageFrameAndSend(subscriptionId, channel, bodyMessage); //connections send the whole channel this message
                checkIfReceiptRequestedAndSend(message);
            }
        }
    }
    
    
    
    private void subscribeProcess(Frame message){
        Set<String> expectedHeaders = new HashSet<String>(Arrays.asList("destination", "id"));
        if(!messageValidityCheck(message, expectedHeaders)){ //message invalid
            createMalFormedErrorFrameAndSendAndDisconnect(message, expectedHeaders); //send error and disconnect
        }
        else{ //message is valid
            String channel = message.getHeaders().get("destination");
            User user = connections.getUserByConnectionId(connectionId);
            if(user.getSubscriptionIdByChannel(channel) != null){ // already subscribed
                System.out.println("USER ALREADY SUBSCRIBED");
                createRegularErrorFrameAndSendAndDisconnect(message, "you are already subscribed"); //send error and disconnect
            }
            else{ // not subscribed
                String subscriptionId = message.getHeaders().get("id");
                connections.subscribe(connectionId, channel, subscriptionId);
                checkIfReceiptRequestedAndSend(message);
            }
        }
    }
    
    
    
    private void unsubscribeProcess(Frame message){
        Set<String> expectedHeaders = new HashSet<String>(Arrays.asList("id"));
        if(!messageValidityCheck(message, expectedHeaders)){ //message invalid
            createMalFormedErrorFrameAndSendAndDisconnect(message, expectedHeaders); //send error and disconnect
        }
        else{ //message is valid
            String subscriptionId = message.getHeaders().get("id");
            User user = connections.getUserByConnectionId(connectionId);
            String channel = user.getChannelBySubscriptionId(subscriptionId);
            if(channel == null){ //subscriptionId doesnt exist--> he is already unsubscribed
                createRegularErrorFrameAndSendAndDisconnect(message, "already unsubscribed"); //send error and disconnect
            }
            else{ //he is subscribed
                connections.unsubscribe(connectionId, channel);
                checkIfReceiptRequestedAndSend(message);
            }
        }
    }
    
    
    
    private void disconnectProcess(Frame message){
        connections.disconnect(connectionId);
        checkIfReceiptRequestedAndSend(message);
    }
    
    //end private processes



    // assistant private methods

    private void createMalFormedErrorFrameAndSendAndDisconnect(Frame message, Set<String> expectedHeaders){
        shouldTerminate = true; //make sure socket closed after sending the error
        HashMap<String, String> headers = new HashMap<String, String>();
        headers.put("message", "malformed frame received");
        String receipt = (String)message.getHeaders().get("receipt");
        if(receipt!=null){ //check if receipt requested, and add this header if needed
            headers.put("receipt-id", receipt);
        }
        Set<String> malformedHeaders = new HashSet<String>();
        for (String expectedHeader : expectedHeaders) {
            if(!headers.containsKey(expectedHeader))
            {
                malformedHeaders.add(expectedHeader);
            }
        }
        String body = "message did not contain: ";
        for (String malformedHeader : malformedHeaders) {
            body+=malformedHeader+", ";
        }
        body = body.substring(0, body.length()-2);
        Frame eror = new Frame("ERROR", headers, body);
        connections.send(connectionId, eror);
    }


    private void createRegularErrorFrameAndSendAndDisconnect(Frame message ,String reason){
        shouldTerminate = true; //make sure socket closed after sending the error
        HashMap<String, String> headers = new HashMap<String, String>();
        headers.put("message", reason); //check if receipt requested, and add this header if needed
        String receipt = (String)message.getHeaders().get("receipt-id");
        if(receipt!=null){
            headers.put("receipt-id", receipt);
        }
        Frame error = new Frame("ERROR", headers, "");
        connections.send(connectionId, error);
        // disconnect user
        disconnectProcess(message);
        
    }

    private void createRegularErrorFrame(Frame message, String reason){
        HashMap<String, String> headers = new HashMap<String, String>();
        headers.put("message", reason); //check if receipt requested, and add this header if needed
        String receipt = (String)message.getHeaders().get("receipt-id");
        if(receipt!=null){
            headers.put("receipt-id", receipt);
        }
        Frame error = new Frame("ERROR", headers, "");
        connections.send(connectionId, error);
    }

    
    
    private void createConnectedFrameAndSend() {
        HashMap<String, String> headers = new HashMap<String, String>();
        headers.put("version", "1.2");
        Frame connectedFrame = new Frame("CONNECTED", headers, "");
        connections.send(connectionId, connectedFrame);
    }

    
    
    private void createMessageFrameAndSend(String senderSubscriptionId, String destination, String body) {
        HashMap<String, String> headers = new HashMap<String, String>();
        headers.put("subscription", senderSubscriptionId);
        headers.put("destination", destination);
        headers.put("message-id", connections.getNextMessageIdAndIncrement()+"");
        Frame message = new Frame("MESSAGE", headers, body);
        connections.send(destination, message);
    }

    
    
    private Frame ReceiptFrameGenerator(String requestedReceiptId) {
        HashMap<String, String> headers = new HashMap<String, String>();
        headers.put("receipt-id", requestedReceiptId);
        return new Frame("RECEIPT", headers, "");
    }

    
    
    private boolean messageValidityCheck(Frame message, Set<String> expectedHeaders){
        Set<String> headersKeysSet = message.getHeaders().keySet();
        return headersKeysSet.containsAll(expectedHeaders);
    }
    
    
    
    private void checkIfReceiptRequestedAndSend(Frame message){
        String receiptId = (String)message.getHeaders().get("receipt-id");
        if(receiptId!=null){
            Frame receiptFrame = ReceiptFrameGenerator(receiptId);
            connections.send(connectionId, receiptFrame);
        }
    }
}

