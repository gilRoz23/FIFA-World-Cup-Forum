package bgu.spl.net.impl.stomp;

import java.util.HashMap;
import java.util.Set;

public class User {
    private String passcode;
    private HashMap<String, String> subscriptionIdToChannel;
    private HashMap<String, String> channelToSubscriptionId;
    private boolean isConnected;
    
    public User(String passcode){
        subscriptionIdToChannel = new HashMap<String, String>();
        channelToSubscriptionId = new HashMap<String, String>();
        this.passcode = passcode;
        this.isConnected = false;
    }

        // methods
    public String getChannelBySubscriptionId(String subscriptionId){
        return subscriptionIdToChannel.get(subscriptionId);
    }
    public String getSubscriptionIdByChannel(String channel){
        return channelToSubscriptionId.get(channel);
    }
    public void subscribe(String subscriptionId, String channel){
        subscriptionIdToChannel.put(subscriptionId, channel);
        channelToSubscriptionId.put(channel, subscriptionId);
    }
    public void unsubscribe(String channel) {
        String subscriptionId = channelToSubscriptionId.remove(channel);
        subscriptionIdToChannel.remove(subscriptionId);
    }
    public boolean isCorrectPasscode(String loginPasscode){
        return this.passcode.equals(loginPasscode);
    }
    public boolean isConnected() {
        return isConnected;
    }
    public void connect() {
        isConnected = true;
    }
    public void disconnect() {
        isConnected = false;
        subscriptionIdToChannel = new HashMap<String, String>();
        channelToSubscriptionId = new HashMap<String, String>();
    }
    public Set<String> getChannels(){
        return channelToSubscriptionId.keySet();
    }
}
