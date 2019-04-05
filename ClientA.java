package TRY2;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowEvent;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.UnknownHostException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Random;
import java.util.StringTokenizer;

import javax.swing.DefaultListModel;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;


public class ClientA extends JFrame implements ActionListener{
    private JButton search;  //Buttons
    private JButton dload;
    private JButton close;  
    private JButton directory; 
    
    private JList jl;   // List that will show found files
    private JLabel label; //Label "File Name
    private JLabel labelADD;
    private JTextField tf,tf2,tfADD; // Two textfields: one is for typing a file name, the other is just to show the selected file
    DefaultListModel listModel; // Used to select items in the list of found files
    public static String Directory;
     ArrayList<String> files;
    Socket socket;
    PrintStream printServer;
    BufferedReader cinFromServer;
    static ServerSocket ClientServer;
    public static int myport;
    public ClientA(){
        super("ClientA");
        setLayout(null);
        setSize(500,600);
        
        label=new JLabel("File name:");
        label.setBounds(50,50, 80,20);
        add(label);
        
        labelADD=new JLabel("Directory:");
        labelADD.setBounds(10,10, 80,20);
        add(labelADD);
        
        tf=new JTextField();
        tf.setBounds(130,50, 220,20);
        add(tf);
        
        tfADD=new JTextField();
        tfADD.setBounds(90,10, 300,20);
        add(tfADD);
        
        search=new JButton("Search");
        search.setBounds(360,50,80,20);
        search.addActionListener(this);
        add(search);
        
        directory=new JButton("Accept");
        directory.setBounds(400,10,80,20);
        directory.addActionListener(this);
        add(directory);     
        
        listModel = new DefaultListModel();
        jl=new JList(listModel);
        
        JScrollPane listScroller = new JScrollPane(jl);
        listScroller.setBounds(50, 80,300,300);
        
        add(listScroller);
        
        dload=new JButton("Download");
        dload.setBounds(200,400,130,20);
        dload.addActionListener(this);
        add(dload);
     
        tf2=new JTextField();
        tf2.setBounds(200,430,130,20);
        add(tf2);
        
        close=new JButton("Close");
        close.setBounds(360,470,80,20);
        close.addActionListener(this);
        add(close);
        
        setVisible(true);
        files = new ArrayList<String>();
        
        try{
        	socket = new Socket("localhost",2846);
        	 // Files information  	
            printServer = new PrintStream(socket.getOutputStream());
            cinFromServer = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            printServer.println("HELLO");
            if(cinFromServer.readLine().equals("HI")){
            	System.out.println("HI");
            	//socket.close();
            	//System.out.println("Error!");
            }
            ClientServer = new ServerSocket(0); 
            myport = ClientServer.getLocalPort();
        }catch (IOException ex){}
        
    }
    
    public void actionPerformed(ActionEvent e){
    	if(e.getSource()==directory){
    		Directory = tfADD.getText();
    		System.out.println(Directory);
    		File folder = new File(Directory);
    		File[] listOfFiles = folder.listFiles();
    		for(int i=0;i<listOfFiles.length;i++)
    		{
    			if(listOfFiles[i].isFile())
    			{
    				String temp = listOfFiles[i].getName();
    				System.out.println(temp);
    				StringTokenizer Tok = new StringTokenizer(temp,".");
    				SimpleDateFormat sdf = new SimpleDateFormat("dd/MM/yy");
    				files.add("<"+Tok.nextToken()+","+Tok.nextToken()+","+listOfFiles[i].length()+","+sdf.format(listOfFiles[i].lastModified())+",127.0.0.1,"+myport+">");
    			}
    		}
    	
    		String msg="";
    		for(int i=0;i<files.size();i++)
    			msg+=files.get(i);
            printServer.println(msg);
    	}
        if(e.getSource()==search){ //If search button is pressed show 25 randomly generated file info in text area 
            listModel.removeAllElements();
        	String fileName=tf.getText();
            printServer.println("SEARCH:"+fileName);
            try {
				String msgFromServer=cinFromServer.readLine();
				StringTokenizer Tok = new StringTokenizer(msgFromServer,"<>");
				if(Tok.nextToken().equals("FOUND:"))
				{
					int i=0;
					while(Tok.hasMoreTokens()){
						 listModel.insertElementAt(fileName+" "+Tok.nextToken(), i);
						 i++;
					 }
				} else listModel.insertElementAt(fileName+" "+msgFromServer,0);
		
			} catch (IOException e1) {
				e1.printStackTrace();
			}
            
           /* Random r=new Random();
            for (int i = 0; i < 25; i++) {
                listModel.insertElementAt(fileName+" "+str[r.nextInt(str.length)],i);*/
        } 
        else if(e.getSource()==dload){   //If download button is pressed get the selected value from the list and show it in text field
            String SelectedValue = jl.getSelectedValue().toString();
            StringTokenizer Tok = new StringTokenizer(SelectedValue,", :");
            String fileName = Tok.nextToken();
            String filetype = Tok.nextToken();
            int filesize = Integer.parseInt(Tok.nextToken());
            String filedate = Tok.nextToken();
            String fileIP = Tok.nextToken();
            int fileport = Integer.parseInt(Tok.nextToken());
            /////////////////////////////////////
            int FILE_SIZE = filesize*2;
            int bytesRead;
    	    int current = 0;
    	    FileOutputStream FOS = null;
    	    BufferedOutputStream BOS = null;
    	    Socket ssock = null;
    	    ////////////////////////////////////
    	    try{
    	    	ssock = new Socket(fileIP,fileport);
    	    	System.out.println("Connecting...");
    	    	PrintStream printPeer = new PrintStream(ssock.getOutputStream());
    	    	printPeer.println("DOWNLOAD: "+fileName+","+filetype+","+filesize);
    	    	byte [] mybytearray  = new byte [FILE_SIZE];
    	    	byte [] firstpart = new byte[6];
    	    	InputStream IS = ssock.getInputStream();
    	    	FOS = new FileOutputStream(Directory+""+fileName+"."+filetype);
    		    BOS = new BufferedOutputStream(FOS);
    		    bytesRead = IS.read(firstpart,0,6);
    		   
    		    if(bytesRead==6){
    		    System.out.println(bytesRead);
    		    	bytesRead = IS.read(mybytearray,0,mybytearray.length);
    		    	System.out.println("DONE:");
        		    current = bytesRead;
        		    System.out.println("DONE:"+current);
        		    do {
        		         bytesRead = IS.read(mybytearray, current, (mybytearray.length-current));
        		         if(bytesRead >= 0) current += bytesRead;
        		         System.out.println("DONE:"+current+"|"+bytesRead);
        		      } while(bytesRead > -1);
        		    System.out.println("DONE2:"+current);
        		    BOS.write(mybytearray, 0 , current);
        		    BOS.flush();
        		    tf2.setText(jl.getSelectedValue().toString()+" donwloaded");
        		    printServer.println("SCORE of "+fileIP+":1");
        		    ssock.close();
    		    } else{
    		    	printServer.println("SCORE of "+fileIP+":0");
    		    	 tf2.setText("NO!");
    		    	ssock.close();
    		    }
    		    	    
    	    }catch(Exception ex){}
        	
        }
        else if(e.getSource()==close){ //If close button is pressed exit
        	printServer.println("BYE");
            System.exit(0);
        }
      
    }
    public static void main(String[] args){
        ClientA ex=new ClientA();
        UploadThread UT = new UploadThread(ClientServer,ClientServer.getLocalPort());
        
		UT.start();
        ex.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE); // Close the window if x button is pressed
        
    }
    
    
}
