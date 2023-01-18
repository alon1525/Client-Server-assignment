package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.*;
import bgu.spl.net.impl.stomp.*;


public class StompServer {

    public static void main(String[] args) {
        if(args[1].contains("tpc"))
        {
        Server.threadPerClient(
            Integer.parseInt(args[0]),//port number make sure it fits the right number.
            () -> new ServerProtocol(), //protocol factory
            FullMessageEncoderDecoder::new //message encoder decoder factory
        ).serve();
        }
        else if(args[1].contains("reactor"))
        {
        Server.reactor(
                Runtime.getRuntime().availableProcessors(),
                Integer.parseInt(args[0]), //port
                () -> new ServerProtocol(), //protocol factory
                FullMessageEncoderDecoder::new //message encoder decoder factory
        ).serve();
        }
    }
}
