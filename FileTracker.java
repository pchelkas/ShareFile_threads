package TRY2;

import java.awt.List;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowEvent;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Random;
import javax.swing.DefaultListModel;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;

import java.io.*;
import java.net.*;

public class FileTracker {
	public static final int port = 2846;
	
	public Hashtable<String, ArrayList<FileInfo>> FILES = new Hashtable<String, ArrayList<FileInfo>>();
	ArrayList<FileInfo> FILEINFO = new ArrayList<FileInfo>();
	public Hashtable<String, FileInfo> PEERS = new Hashtable<String, FileInfo>();
	
    public FileTracker(){
        
    }
  
    public static void main(String[] args) throws IOException {
    	new FileTracker().runFileTracker();
    }
    
	public void runFileTracker() throws IOException{
		// TODO Auto-generated method stub
		ServerSocket FTSocket = new ServerSocket(port);
		System.out.println("Server is running...");
		while(true){
		Socket socket = FTSocket.accept();
		FileTrackerThread temp = new FileTrackerThread(socket,this);
		temp.start();
	}
}
}
