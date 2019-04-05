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
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Scanner;
import java.util.StringTokenizer;





public class UploadThread extends Thread{
	public final static String SERVER = "127.0.0.1"; 
	
	Socket sock; 
	String name;
	int localport;
	Socket mainsock;
	ServerSocket ClientServer;
	String group = null;
	
	
	UploadThread(ServerSocket ClientServer, int localport){
		this.ClientServer = ClientServer;
		this.localport=localport;
	}
	
	
	public void run() {
		try {
	
	while(true) {
		sock = ClientServer.accept();
		 ShareThread st = new ShareThread(sock);
	     st.start();
	}
	
	// BufferedReader fromPeer = new BufferedReader(new InputStreamReader(sock.getInputStream())); 
	// PrintStream toPeer = new PrintStream(sock.getOutputStream());

	//String request = fromPeer.readLine();
	
	
	
	}  catch(IOException ex){
		ex.printStackTrace();
		
}
	}
}
