package TRY2;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Scanner;
import java.util.StringTokenizer;


public class FileTrackerThread extends Thread {
	
	Socket socket;
	FileTracker FileTracker;
	boolean status; 
	ArrayList<String> enteredFiles = new ArrayList<String>();
	ArrayList<FileInfo> enteredFileInfo = new ArrayList<FileInfo>();
	
	public FileTrackerThread(Socket socket, FileTracker FT){
		this.socket = socket;
		FileTracker = FT;
		status = false;
	}
	
	public void run(){
		try{
			 String message = null;
			 BufferedReader cinFromClient = new BufferedReader(new InputStreamReader(socket.getInputStream()));
			 PrintStream coutToClient = new PrintStream(socket.getOutputStream());
			 String[] ip = socket.getInetAddress().toString().split("/");
 			 FileTracker.PEERS.put(ip[1], new FileInfo(0,0));
			 System.out.println("New connection:"+ip[1]+"|"+socket.getPort());
			 
			 ////////////////SAY HELLO///////////////////
			 while(true)
			 {
			 message = cinFromClient.readLine();
			 System.out.println(message);
			 if(message.equals("HELLO")){
				 coutToClient.println("HI");
				 break;
			 } else coutToClient.println("Say HELLO!");
			 }
			 //////////READING FILES//////////
			 while(true){
				 message = cinFromClient.readLine();
				 System.out.println(message);
				 if(message.equals("BYE"))
				 {
					 socket.close();
					 break;
				 }
				 if(message.contains("<") && message.contains(">"))
				 {
					 StringTokenizer Tok = new StringTokenizer(message,"<,>");
					 while(Tok.hasMoreTokens()){
						 String filename = Tok.nextToken();
						 String filetype = Tok.nextToken();
						 int filesize = Integer.parseInt(Tok.nextToken());
						 String filedate = Tok.nextToken();
						 String ipaddress = Tok.nextToken();
						 int fileport = Integer.parseInt(Tok.nextToken());
						 System.out.println(filename+" "+filetype+" "+filesize+" "+filedate+" "+ipaddress+" "+fileport);
						 if(FileTracker.FILES.containsKey(filename))
						 {
							 enteredFiles.add(filename);
							 enteredFileInfo.add(new FileInfo(filetype,filesize,filedate,ipaddress, fileport));
							 FileTracker.FILES.get(filename).add(new FileInfo(filetype,filesize,filedate,ipaddress, fileport));
						 }else {
							 ArrayList<FileInfo> newInfo = new ArrayList<FileInfo>();
							 newInfo.add(new FileInfo(filetype,filesize,filedate,ipaddress, fileport));
							 FileTracker.FILES.put(filename, newInfo);
							 enteredFiles.add(filename);
							 enteredFileInfo.add(new FileInfo(filetype,filesize,filedate,ipaddress, fileport));
						 }
					 }
					 break;
				 } 
			 }
			 System.out.println("Done!");
			 
			 ///////////////////////////////////////////
			 while(true){
				 message = cinFromClient.readLine();
				 System.out.println(message);
						if(message.equals("BYE"))
						{
							//NEED UPDATE
							for(int i=0;i<enteredFiles.size();i++)
							{
								if(FileTracker.FILES.get(enteredFiles.get(i)).size()==1)
									FileTracker.FILES.remove(enteredFiles.get(i));
								else
								FileTracker.FILES.get(enteredFiles.get(i)).remove(enteredFileInfo.get(i));
							}
							socket.close();
							break;
						}
						String[] parts = message.split(":");
						if(parts[0].equals("SEARCH"))
						{
						   if(FileTracker.FILES.containsKey(parts[1]))
						   {
							   ArrayList<FileInfo> temp = FileTracker.FILES.get(parts[1]);
							   String FOUND = "FOUND:";
							   for(int i=0;i<temp.size();i++)
							   {
								   int NOR = FileTracker.PEERS.get(temp.get(i).getIp()).getNumOfRequests();
								   int NOU = FileTracker.PEERS.get(temp.get(i).getIp()).getNumOfUploads();
								   int persentage = 0; 
								   if(NOR==0) persentage = 0;
								   else persentage = NOU/NOR*100;
								  
								   FOUND+="<"+temp.get(i).getFormat()+","+temp.get(i).getSize()+","+
								   temp.get(i).getDate()+","+temp.get(i).getIp()+","+temp.get(i).getPort()+":"+
								   persentage+"%>";
								   coutToClient.println(FOUND);
							   }
						   } else {
							   coutToClient.println("NOT FOUND");
						   }
						}else{
							int bonus = Integer.parseInt(parts[1]);
							String[] parts2 = parts[0].split(" ");
							String ipToUpdate = parts2[2];
							FileTracker.PEERS.get(ipToUpdate).update(bonus);
						}
						
			 }
			 
		}catch(Exception ex){ex.printStackTrace();}
	}
	
}
