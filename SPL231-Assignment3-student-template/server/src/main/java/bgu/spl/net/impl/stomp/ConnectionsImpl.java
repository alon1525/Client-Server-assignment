package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.*;

public class ConnectionsImpl implements Connections<String> {
    
    public List<String> topics;                     //List of All Exhisting Topics
    public List<List<Integer>> subscriptionsList;                     //For each topic, saves all Subscription ID's (Indeces by Topics)
    public Dictionary<Integer,List<Integer>> userDictionary;          //For each Connection ID, saves all Subscription ID
    public List<String> existingAccounts;                     //List of All usernames
    public List<List<Object>> accountInfo;                    //For each Connection ID, saves info (Activeness,username, password) - Indeces by existingAccounts
    public ConcurrentHashMap<Integer, ConnectionHandler<String>> ID_ConnectionHandler;         //For each user, saves the Handlers' ID
    Integer numberOfConnections = 0;
    

    public ConnectionsImpl()
    {
        topics = new LinkedList<String>();
        subscriptionsList = new LinkedList<List<Integer>>();
        userDictionary = new Hashtable<Integer,List<Integer>>();
        existingAccounts = new LinkedList<String>();
        accountInfo = new LinkedList<List<Object>>();
        
        ID_ConnectionHandler = new ConcurrentHashMap<>();
    }

    public String getuserName(int index)
    {
        return existingAccounts.get(index);
    }

    public int indexName(String loginName)
    {
        return existingAccounts.indexOf(loginName);
    }

    public String[] getDetails(int index)
    {
        String[] ans = new String[2];
        ans[0] = (String)accountInfo.get(index).get(1);
        ans[1] = (String)accountInfo.get(index).get(2);
        return ans;
    }

    public int getSubscriptionId(int connectionId,String topic)
    {
        int index = topics.indexOf(topic);
        List<Integer> topicsubs = subscriptionsList.get(index);
        List<Integer> subIds = userDictionary.get(connectionId);
        for(Integer sub : subIds)
        {
            if(topicsubs.contains(sub))
            {
                    
                return sub;
            }
        }
        return -1;
    }
    

    public boolean isClientActive(int index)
    {
        return (boolean)accountInfo.get(index).get(0);
    }

    public void switchActivity(int index)
    {
        boolean bool = !(boolean)accountInfo.get(index).get(0);
        accountInfo.get(index).set(0, bool);
    }

    public synchronized int addNewConnectionHandler(ConnectionHandler ch)
    {
        synchronized(numberOfConnections){
            ID_ConnectionHandler.put(numberOfConnections, ch);
            numberOfConnections++;
            return numberOfConnections-1;
        }
    }

    public void addClient(String login,String passcode, int connectionId)
    {
        existingAccounts.add(connectionId,login);
        List<Object> newInfo = new LinkedList<Object>();
        newInfo.add(connectionId);
        newInfo.add(passcode);
        newInfo.add(login);
        newInfo.add(true);
        accountInfo.add(connectionId, newInfo);
        List<Integer> empty = new LinkedList<Integer>();
        userDictionary.put(connectionId, empty); 
        
    }

    public boolean send(int connectionId, String msg)
    {
        if(!ID_ConnectionHandler.keySet().contains(connectionId)){
            return false;
        }
        else{
            ID_ConnectionHandler.get(connectionId).send(msg);
            return true;
        }
    }

    public void send(String channel, String msg)
    {
        int index = topics.indexOf(channel);
        for(Integer member : subscriptionsList.get(index))
        {
            send(member, msg);
            //check what to do if send == false
        }
    }

    public synchronized boolean subscribe(String topic, int subscriptionId, int connectionId)
    {
        if(!topics.contains(topic))
        {
            topics.add(topics.size(), topic);
            List<Integer> newTopic = new LinkedList<Integer>();
            newTopic.add(subscriptionId);
            subscriptionsList.add(topics.size(), newTopic);
        }
        else
        {
            int indexOfTopic = topics.indexOf(topic);
            if(subscriptionsList.get(indexOfTopic).contains(subscriptionId)){
                
                return false;   
            }
            else
            {
                subscriptionsList.get(indexOfTopic).add(subscriptionId);
            }
        }
        List<Integer> toAdd = (List<Integer>)userDictionary.get(connectionId);
        toAdd.add(subscriptionId);
        return true;
    }

    public synchronized boolean unsubscribe(int subscriptionId,int connectionId)
    {
        List<Integer> mySubscriptions = (List<Integer>)userDictionary.get(connectionId);
        if(mySubscriptions != null && mySubscriptions.remove((Integer)subscriptionId))
        {
            for(List<Integer> list : subscriptionsList)
            {
                if(list.contains(subscriptionId))
                {
                    list.remove(subscriptionId);
                    return true;
                }
            }
            return false;
        }
        else
        {
            return false;
        }
    }

    public void disconnect(int connectionId)
    {
        if(ID_ConnectionHandler.containsKey(connectionId)){             //Remove from Handler List
            ID_ConnectionHandler.remove(connectionId);}
        List<Integer> mySubscriptions = (List<Integer>)userDictionary.remove(connectionId);     //Remove client from dictionary
        if(mySubscriptions != null){                                    //Remove each subs. in dictionary from Sub. List
            for(Integer SubscriptionID : mySubscriptions)
            {
                for(List<Integer> list:subscriptionsList)
                {
                    list.remove((Integer)SubscriptionID);
                }
            }
        }
        for(int i = 0;i<accountInfo.size();i++)                         //Change Activation boolean to false
        {
            List<Object> current = accountInfo.get(i);
            if((int)current.get(3) == connectionId)
            {
                current.set(0, false);
            }
        }
    }
}
