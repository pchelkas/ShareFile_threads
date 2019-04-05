package TRY2;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Scanner;
import java.util.StringTokenizer;
import java.util.Random;





public class ShareThread extends Thread{
	public final static String SERVER = "127.0.0.1"; 
	public String Directory = ClientA.Directory;
	public String FILE_TO_SEND;
	 
	Socket SocketToSend;
	Socket SocketToGet;
	Socket sock; 
	String name;
	int localport;
	
	FileInputStream fis = null;
    BufferedInputStream bis = null;
    OutputStream os = null;
	
	
	String group = null;
	
	
	ShareThread( Socket socket2){
		
		sock = socket2;
	}
	
	public void run() {
		try {
		 BufferedReader fromPeer = new BufferedReader(new InputStreamReader(sock.getInputStream())); 
		 String request = fromPeer.readLine();
		 StringTokenizer st = new StringTokenizer(request);
		 String[] tokens;
		 String[] tokens2;
		 String[] tokens3;
		 st.nextToken();
		 tokens = st.nextToken().split(",");   // to get file name and type
		 
		 FILE_TO_SEND = Directory+tokens[0]+"."+tokens[1];
		 
		 PrintStream toPeer = new PrintStream(sock.getOutputStream()); // writer to server
		 Random rand = new Random();
		 int randomNum = rand.nextInt((100 - 1) + 1) + 1;
		 System.out.println(randomNum);
		 if(randomNum>=50) {
			 toPeer.println("NO!");
			 
		 }
		 else {
			 while(true){
				 try{
					 File myFile = new File (FILE_TO_SEND);
					 byte [] mybytearray  = new byte [(int)myFile.length()];
			          fis = new FileInputStream(myFile);
			          bis = new BufferedInputStream(fis);
			          bis.read(mybytearray,0,mybytearray.length);
			          os = sock.getOutputStream();
			          System.out.println("Sending " + FILE_TO_SEND + "(" + mybytearray.length + " bytes)");
			          toPeer.print("FILE: ");
			          os.write(mybytearray,0,mybytearray.length);
			          os.flush();
			          System.out.println("Done.");
				 } finally{
					 if (bis != null) bis.close();
			          if (os != null) os.close();
			          if (sock!=null) sock.close();
			          break;
				 }
			 }
			 
	
	}
		}
		catch(IOException ex){
			ex.printStackTrace();
		
		
	}
	
	
	
	
	
}}