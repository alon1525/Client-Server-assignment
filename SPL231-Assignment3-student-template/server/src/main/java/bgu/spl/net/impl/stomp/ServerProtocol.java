package bgu.spl.net.impl.stomp;
import java.time.LocalDateTime;
import java.util.LinkedList;
import java.util.List;

import bgu.spl.net.api.*;
import bgu.spl.net.srv.Connections;

public class ServerProtocol implements MessagingProtocol<String> {
    private boolean shouldTerminate = false;
    private int connectionId;
    private Connections<String> connections;
    private Integer messageId = 0;

    
    

    @Override
    public String process(String msg) {
        List<Object> msgData = classifyMessage(msg);
        if(msgData.get(0)=="CONNECT"){
            int end = msg.indexOf("\n", (int)msgData.get(2));
            String login = msg.substring((int)msgData.get(2), end);
            end = msg.indexOf("\n", (int)msgData.get(4));
            String passcode = msg.substring((int)msgData.get(4), end);

            int clientIndex = connections.indexName(login);
            if(clientIndex != -1)
            {
                String[] details = connections.getDetails(end);
                if(passcode.equals(details[1]))
                {
                    if(connections.isClientActive(clientIndex))
                    {
                        String error = "ERROR\n" + "receipt-id: 8974" + "message: malformed frame recieved\n" + "the message:\n" + "-----\n" + msg + "\n-----\n" + "theres an account with same information that is already logged in" ; 
                        connections.send(connectionId,error);
                    }
                    else
                    {
                        connections.switchActivity(clientIndex);
                        connections.send(connectionId,"CONNECTED"+ "\n" + "version:1.2" + "\n" + "\n" + "\u0000");
                    }
                }
                else
                {
                    String error = "ERROR\n" + "receipt-id: 8974" + "message: malformed frame recieved\n" + "the message:\n" + "-----\n" + msg + "\n-----\n" + "wrong password" ; 
                        connections.send(connectionId,error);
                }
            }
            else
            {
                connections.addClient(login, passcode, connectionId);
                connections.send(connectionId,"CONNECTED"+ "\n" + "version:1.2" + "\n" + "\n" + "\u0000");
            }
        }
        else if(msgData.get(0)=="DISCONNECT"){
            shouldTerminate = true;
            int end = msg.indexOf("\n", (int)msgData.get(2));
            String recieptId = msg.substring((int)msgData.get(2), end);
            String toSend = "RECEIPT" +"\n" + "receipt-id:" + recieptId + "\n" + "\n" + "\u0000";
            connections.send(connectionId,toSend);
            connections.disconnect(connectionId);
        }
        else if(msgData.get(0)=="SUBSCRIBE")
        {
            int end = msg.indexOf("\n", (int)msgData.get(4));
            String id = msg.substring((int)msgData.get(4), end);
            end = msg.indexOf("\n", (int)msgData.get(2));
            String destination = msg.substring((int)msgData.get(2), end);
            connections.subscribe(destination, Integer.parseInt(id), connectionId);
            end = msg.indexOf("\n", (int)msgData.get(6));
            String receiptId = msg.substring((int)msgData.get(6), end);
            String receipt = "RECEIPT" + "\n" + "receipt-id:" + receiptId+ "\n" + connections.getSubscriptionId(connectionId, destination) + "\n" + destination + "\n" + "\u0000";
            connections.send(connectionId, receipt);
        }
        else if(msgData.get(0)=="UNSUBSCRIBE")
        {
            int end = msg.indexOf("\n", (int)msgData.get(2));
            String id = msg.substring((int)msgData.get(2), end);
            connections.unsubscribe(Integer.parseInt(id), connectionId);
        }
        else if(msgData.get(0)=="SEND")//need to write a proper message
        {
            int end = msg.indexOf("\n\u0000");
            String message = msg.substring((int)msgData.get(3), end);
            end = msg.indexOf("\n", (int)msgData.get(2));
            String destination = msg.substring((int)msgData.get(2), end);
            int subId = connections.getSubscriptionId(connectionId, destination);
            if(subId!=-1)
            {
                String finalMessage = "MESSAGE\n" + "subscription:" + subId + "\n" + "message-id:" + messageId + "\n" + "destination:/topic/" + destination + "\n\n" + connections.getuserName(connectionId) + "\n" + message + "\n\u0000";
                messageId++;
                connections.send(destination,finalMessage);
            }
            else
            {
                String error = "ERROR\n" + "receipt-id: 8974" + "message: malformed frame recieved\n" + "the message:\n" + "-----\n" + msg + "\n-----\n" + "cant send a message to topic you are not subscribe to" ; 
                        connections.send(connectionId,error);
            }
            
        }
        System.out.println("[" + LocalDateTime.now() + "]: " + msg);
        return "s";
    }

    public void start(int connectionId, Connections<String> connections)
    {
            this.connectionId = connectionId;
            this.connections = connections;

    }


    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    private List<Object> classifyMessage(String msg)
    {
        List<Object> msgData = new LinkedList<Object>();
        int index = msg.indexOf("\n");
        String firstLine = msg.substring(0, index);
        if(firstLine.equals("DISCONNECT"))
        {
            msgData.add("DISCONNECT");
            msgData.add("receipt");
            msgData.add(msg.indexOf("receipt")+8);
        }
        else if(firstLine.equals("UNSUBSCRIBE"))
        {
            msgData.add("UNSUBSCRIBE");
            msgData.add("id");
            msgData.add(msg.indexOf("id")+3);
        }
        else if(firstLine.equals("SEND"))
        {
            msgData.add("SEND");
            msgData.add("destination");
            msgData.add(msg.indexOf("destination")+12);
            index = msg.indexOf("\n\n") + 4;
            msgData.add(index);
            
        }
        else if(firstLine.equals("CONNECT"))//maybe delete unnecessary info
        {
            msgData.add("CONNECT");
            msgData.add("accept-version");
            msgData.add(msg.indexOf("accept-version")+15);
            msgData.add("login");
            msgData.add(msg.indexOf("login")+6);
            msgData.add("host");
            msgData.add(msg.indexOf("host")+5);
            msgData.add("passcode");
            msgData.add(msg.indexOf("passcode")+9);
        }
        else if(firstLine.equals("SUBSCRIBE"))
        {
            msgData.add("SUBSCRIBE");
            msgData.add("destination");
            msgData.add(msg.indexOf("destination")+13);
            msgData.add("id");
            msgData.add(msg.indexOf("id")+3);
            msgData.add("receipt");
            msgData.add(msg.indexOf("receipt")+8);
        }
        
        
        return msgData;
    }


}
