import java.io.*;
import java.net.*;

public class Client {
    public static void main(String[] argv) {

        if (argv.length != 2) {
            System.err.println("Uso corretto: java Client server porta");
            System.exit(1);
        }

        try{
            BufferedReader fromUser = new BufferedReader(new InputStreamReader(System.in));

            System.out.println("Inserisci il tuo indirizzo email: ");
            String email = fromUser.readLine();

            Socket s = new Socket(argv[0], Integer.parseInt(argv[1]));

            System.out.println("Inserisci la password: ");
            String pwd = fromUser.readLine();

            BufferedReader fromServer = new BufferedReader(new InputStreamReader(s.getInputStream(),"UTF-8"));
            BufferedWriter toServer = new BufferedWriter(new OutputStreamWriter(s.getOutputStream(),"UTF-8"));

            toServer.write(email);
            toServer.flush();

            String buff = fromServer.readLine();

            if(!buff.equals("ack")){
                System.err.println("Errore lettura Ack dal server");
                System.exit(2);
            }

            toServer.write(pwd);
            toServer.flush();

            while ((buff = fromServer.readLine()) != null) {
                System.out.println(buff);
            }

            s.close();
        }
        catch(IOException e){
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(100);
        }
    }
}
