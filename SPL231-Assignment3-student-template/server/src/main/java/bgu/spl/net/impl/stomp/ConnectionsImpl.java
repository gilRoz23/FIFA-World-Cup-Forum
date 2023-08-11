package bgu.spl.net.impl.stomp;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.concurrent.atomic.AtomicInteger;

import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;


public class ConnectionsImpl<T> implements Connections<T> {
    
    private HashMap<Integer, ConnectionHandler<T>> connectionIdToConnectionHandler;
    private HashMap<String, List<Integer>> channelToConnectionIdsList;
    private HashMap<Integer, String> connectionIdToUsername;
    private HashMap<String, User> usernameToUser;
    // private AtomicInteger nextMessageId;
    private int nextMessageId;

    private static class ConnectionsInitializerClass<T> {
        private static ConnectionsImpl instance = new ConnectionsImpl();
    }

    private ConnectionsImpl(){
        this.connectionIdToConnectionHandler = new HashMap<Integer, ConnectionHandler<T>>();
        this.channelToConnectionIdsList = new HashMap<String, List<Integer>>();
        this.connectionIdToUsername = new HashMap<Integer, String>();
        this.usernameToUser = new HashMap<String, User>();
        // this.nextMessageId.set(0);
        this.nextMessageId = 0;
    }

    public static ConnectionsImpl getConnectionsInstance(){
        return ConnectionsInitializerClass.instance;
    }

    public void send(int connectionId, T msg){
        ConnectionHandler<T> CH = getConnectionHandlerByConnectionId(connectionId); //get the connection handler
        System.out.println(CH);
        CH.send(msg); //tell the specific connectionHandler to send this msg to its client
    }

    public void send(String channel, T msg){
        List<Integer> connectionIdsList = getConnectionIdsListByChannel(channel);
        for (Integer connectionId : connectionIdsList) {
            ConnectionHandler<T> currentCH = getConnectionHandlerByConnectionId(connectionId);
            currentCH.send(msg);
        }
    }

    public void disconnect(int connectionId){
        User user = getUserByConnectionId(connectionId);
        Set<String> usersChannels = user.getChannels();
        for (String channel : usersChannels) { //for each channel remove the connectionId from its connectionIds list
            System.out.println("channel is: " + channel + " connection id is: " + connectionId);
            channelToConnectionIdsList.get(channel).removeIf(e -> e.equals(connectionId));
        }
        connectionIdToUsername.remove(connectionId); //remove corelation between this connectionId to the username
        user.disconnect(); //disconnect user(sign as disconnected and clean his subscriptions) // remember it needs to be last, because we clean the subscriptions!
    }

    // **added methods**

    // getters
    public List<Integer> getConnectionIdsListByChannel(String Channel){
        List<Integer> emptyIdList = channelToConnectionIdsList.get(Channel);
        if(emptyIdList == null){
            emptyIdList = new LinkedList<Integer>();
            channelToConnectionIdsList.put(Channel, emptyIdList);
        }
        return emptyIdList;
    }
    public ConnectionHandler<T> getConnectionHandlerByConnectionId(Integer connectionId){
        return connectionIdToConnectionHandler.get(connectionId);
    }
    public String getUsernameByConnectionId(Integer connectionId){
        return connectionIdToUsername.get(connectionId);
    }    
    public User getUserByUsername(String username){
        return usernameToUser.get(username);
    }    
    // end getters

    public void addClient(Integer connectionId, ConnectionHandler<T> CH){ //map between connectionId to CH
        connectionIdToConnectionHandler.put(connectionId, CH);
    }

    public void connect(Integer connectionId, String userName){
        connectionIdToUsername.put(connectionId, userName); //connect it
        User user = getUserByUsername(userName);
        user.connect();
    }
    public Integer getNextMessageIdAndIncrement(){
        int sendId = nextMessageId;
        nextMessageId++;
        return sendId;
        // return nextMessageId.getAndIncrement();
    }

    public void subscribe(Integer connectiondId, String channel, String subscriptionId){
        User user = getUserByConnectionId(connectiondId);
        user.subscribe(subscriptionId, channel);
        getConnectionIdsListByChannel(channel).add(connectiondId);
    }


    public User getUserByConnectionId(Integer connectionId){
        String username = getUsernameByConnectionId(connectionId);
        return getUserByUsername(username);
    }
    
    
    public void registerUser(String username, User user) {
        usernameToUser.put(username, user);
    }
    
    
    public void unsubscribe(Integer connectiondId, String channel){
        User user = getUserByConnectionId(connectiondId);
        user.unsubscribe(channel);
        getConnectionIdsListByChannel(channel).remove(connectiondId);
    }
}
