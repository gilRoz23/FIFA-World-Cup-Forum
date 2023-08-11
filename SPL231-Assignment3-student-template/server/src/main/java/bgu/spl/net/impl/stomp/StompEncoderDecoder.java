package bgu.spl.net.impl.stomp;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Set;

import bgu.spl.net.api.MessageEncoderDecoder;

public class StompEncoderDecoder implements MessageEncoderDecoder<Frame> {

    private byte[] bytes = new byte[1 << 10]; //start with 1k
    private int len = 0;

    @Override
    public Frame decodeNextByte(byte nextByte) {
        //stop when the conventional finish character by the stomp protocol is shown
        if (nextByte == '\u0000') {
            return popStringAndExtract();
        }

        pushByte(nextByte);
        return null; //not a line yet
    }

    @Override
//    public byte[] encode(String message) {
//        return (message).getBytes(); //uses utf8 by default
//    }
    public byte[] encode(Frame answerFrame) {
        String stringAnswer = "";
        stringAnswer+= answerFrame.getCommand()+"\n"; //add the command in the first line
        Set<String> headersKeys = answerFrame.getHeaders().keySet();
        String headersString = "";
        for (String key : headersKeys) {
            String value = (String)answerFrame.getHeaders().get(key);
            headersString+=(key+":"+ value+"\n"); //add--> header:value\n to the temp string
        }
        stringAnswer+=headersString; //add the headers temp string
        stringAnswer+="\n"; //the blank line that separates the headers from the body
        //if body isnt empty add it
        if(answerFrame.getBody().length() != 0)
            stringAnswer+=answerFrame.getBody();
        stringAnswer+= "\u0000"; //add end of message convention

        // System.out.println("the encode encoded and thats what i got: " + stringAnswer);
        
        return (stringAnswer).getBytes(); //uses utf8 by default
    }

    private void pushByte(byte nextByte) {
        if (len >= bytes.length) {
            bytes = Arrays.copyOf(bytes, len * 2);
        }

        bytes[len++] = nextByte;
    }

    private Frame popStringAndExtract() {
        //notice that we explicitly requesting that the string will be decoded from UTF-8
        //this is not actually required as it is the default encoding in `ava.
        String frameAsString = new String(bytes, 0, len, StandardCharsets.UTF_8);
        len = 0;
        return extractFrame(frameAsString);
    }

    private Frame extractFrame(String frameAsString){

        System.out.println("I AM AT THE extractFrame() in the encdec with the frame:\n" + frameAsString);
        System.out.println("**** END OF FRAME ******");

        
        String[] beforeBodyAndBody = frameAsString.split("\n\n");
        String[] beforeBodySplittedLines = beforeBodyAndBody[0].split("\n");
        String body = "";
        if (beforeBodyAndBody.length == 2){ // means we have a body
            body = beforeBodyAndBody[1];
        }


        // String[] frameLines = frameAsString.split("\n"); //split by \n (lines)
        // String command = frameLines[0]; // according to protocol first line is command
        HashMap<String, String> headers = new HashMap<String, String>(); 
        int i = 1;
        // while( i<frameLines.length && frameLines[i]!=""){ // according to protocol next lines are headers
        while (i < beforeBodySplittedLines.length) {
            String[] headerAndValue = beforeBodySplittedLines[i].split(":"); //split the headers lines by ":" (header:value)
            String header = headerAndValue[0];
            System.out.println("Header is: " + header);
            String value = headerAndValue[1];
            System.out.println("value is: " + value);
            headers.put(header, value); //add to hashe map key=header, value=value
            i++;
        }
        // //if ther's a body i is now blank line; if there isnt i is out of bound(=frameLines.length)
        // i++; //if ther's a body i is on first line; if there isnt i is out of bound(=frameLines.length +1)
        // String body = "";
        // while(i < frameLines.length){ // i is blank line
        //     body += frameLines[i] + "\n";
        //     i++; //get to the next line
        // }
        // return new Frame(command, headers, body);
        return new Frame(beforeBodySplittedLines[0], headers, body);
    }
}
