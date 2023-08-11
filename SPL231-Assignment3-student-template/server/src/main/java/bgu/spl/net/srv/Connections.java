package bgu.spl.net.srv;

public interface Connections<T> {

    void send(int connectionId, T msg);

    void send(String channel, T msg);

    void disconnect(int connectionId);
}
