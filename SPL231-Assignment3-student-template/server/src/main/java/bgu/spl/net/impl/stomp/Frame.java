package bgu.spl.net.impl.stomp;
import java.util.HashMap;
public class Frame {

    private String command;
    private HashMap<String, String> headers;
    private String body;
    public Frame(String command, HashMap<String, String> headers, String body){
        this.command = command;
        this.headers = headers;
        this.body = body;
    }
    public String getCommand() {
        return command;
    }
    public HashMap<String, String> getHeaders() {
        return headers;
    }
    public String getBody() {
        return body;
    }
}