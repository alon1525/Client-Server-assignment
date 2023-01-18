package bgu.spl.net.srv;

import java.io.IOException;

public interface Connections<T> {

    boolean send(int connectionId, T msg);

    void send(String channel, T msg);

    void disconnect(int connectionId);

    boolean subscribe(String topic, int subscriptionId, int connectionId);

    boolean unsubscribe(int subscriptionId,int connectionId);

    int indexName(String loginName);

    String[] getDetails(int index);

    boolean isClientActive(int index);

    void addClient(String login,String passcode, int connectionId);

    void switchActivity(int index);

    int addNewConnectionHandler(ConnectionHandler<String> ch);

    int getSubscriptionId(int connectionId,String topic);

    String getuserName(int index);
}
