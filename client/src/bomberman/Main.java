package bomberman;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.Socket;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Scanner;
import java.util.concurrent.atomic.AtomicBoolean;

import static java.lang.System.in;

public class Main
{
    static boolean DEBUG = true;
    OutputStream out;
    static char DELIMITER = '\n';
    static char ESCAPE = '\\';
    String session = null;
    AtomicBoolean inLobby = new AtomicBoolean(false);
    String lobbyName = "";
    AtomicBoolean inGame = new AtomicBoolean(false);
    AtomicBoolean running = new AtomicBoolean(true);
    AtomicBoolean connected = new AtomicBoolean(false);
    HashMap<String,Integer> cards = new HashMap<>();
    String host;
    int port;
    Socket clientSocket = null;
    BufferedReader br = null;

    public static void main(String[] args) throws IOException
    {
        new Main();
    }

    Main() throws IOException
    {
        Scanner scanner = new Scanner(in);

        while(clientSocket == null)
        {
            try
            {
                if (DEBUG)
                {
                    clientSocket = new Socket("localhost", 10001);
                    host = "localhost";
                    port = 10001;
                }
                else
                {
                    System.out.print("Addres: ");
                    host = scanner.next();

                    System.out.print("Port: ");
                    port = scanner.nextInt();
                    if (port >= 65536 || port == 0)
                    {
                        System.out.println("Wrong port");
                        return;
                    }
                    clientSocket = new Socket(host, port);
                }
                br = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));

            }
            catch (Exception e)
            {
                System.out.println("Cant connect to server!");
                if (DEBUG)
                    return;
            }

        }

        System.out.print("Nickname: ");
        while(session == null)
        {
            String s;
            String nick = scanner.next().trim();
            if (nick.charAt(0) == '_' && nick.length() > 1)
            {
                s = "SESSION " + nick.substring(1) + DELIMITER;
            }
            else
                s = "LOGIN " + nick + DELIMITER;

            try
            {
                out = clientSocket.getOutputStream();
                send(s);
                System.out.println("Logging...");
                session = br.readLine();
            }
            catch (Exception e)
            {
                System.err.println("Server unexpectly closed connection!");
                return;
            }

            if (session == null)
            {
                System.out.print("Nick is already taken, try another one: ");
                if (!reconnect())
                    return;
            }
        }

        connected.set(true);
        InputThread inputThread = new InputThread(this);
        inputThread.start();

        String msg = "";
        while (running.get())
        {
            msg = br.readLine();
            if (msg == null && !attemptToReconnect())
            {
                if (running.get())
                    System.err.println("Cant reconnect!");
                running.set(false);
                return;
            }

            if (msg == null)
                continue;

            if (DEBUG)
                System.out.println("RECV: "+msg);

            if (!parseMessage(msg))
            {
                System.out.println("Recieved unknown message, make sure that your server and client version match!");
                break;
            }
        }
    }

    private boolean reconnect()
    {
        try
        {
            clientSocket = new Socket(host, port);
            br = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
            out = clientSocket.getOutputStream();
        }
        catch (Exception ex)
        {
            System.out.println("Cant connect to server!");
            return false;
        }
        return true;
    }
    private boolean attemptToReconnect()
    {
        if (!running.get())
            return false;

        connected.set(false);
        for (int i = 0; i < 15; i++)
        {
            System.out.println((i+1)+"/15 attempt to reconnect...");
            try
            {
                clientSocket = new Socket(host, port);
                out = clientSocket.getOutputStream();
                String s = "SESSION "+session+DELIMITER;
                out.write(s.getBytes());
                br = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
                s = br.readLine();
                if (s != null)
                {
                    connected.set(true);
                    return true;
                }
            }
            catch (Exception e)
            {
                // do nothing
            }

            try
            {
                Thread.sleep(1000);
            } catch (InterruptedException e)
            {
                return false;
            }
        }
        return false;
    }

    private boolean parseMessage(String msg)
    {
        String[] parts = split(msg);
        if (parts.length < 1)
            return false;

        switch (parts[0])
        {
            case "LOBBY_LIST":
                parseLobbyList(parts);
                break;
            case "CREATE_LOBBY":
                createLobby(parts[1]);
                break;
            case "JOIN_LOBBY":
                createLobby(parts[1]);
                break;
            case "LEAVE_LOBBY":
                leaveLobby();
                break;
            case "START_GAME":
                startGame();
                break;
            case "MESSAGE":
                String s = "";
                for (int i = 1; i < parts.length; i++)
                    s += parts[i]+" ";
                System.out.println(s.trim());
                break;
            case "FF":
                ff();
                break;
            case "CARD":
                newCard(parts);
                break;
            case "PLAY":
                played(parts);
                break;
        }

        return true;
    }

    private void ff()
    {

    }

    private void played(String[] parts)
    {

    }

    private void newCard(String[] parts)
    {

    }

    private void startGame()
    {
        send("START_GAME"+DELIMITER);
    }

    String[] split(String text)
    {
        ArrayList<String> messages = new ArrayList<>();
        while (!text.isEmpty())
        {
            boolean escaped = false;
            int i = 1;
            for (i = 1; i < text.length(); i++)
            {
                if (text.charAt(i) == ' ' && !escaped)
                    break;

                if (text.charAt(i) == ESCAPE && !escaped)
                    escaped = true;
                else
                    escaped = false;
            }

            if (i >= 1)
                messages.add(text.substring(0, i));
            if (i + 1 <= text.length())
                text = text.substring(i + 1);
            else
                text = "";
        }
        return messages.toArray(new String[messages.size()]);
    }

    String escapeString(String str)
    {
        String msg = "";
        String[] tokens = split(str.trim());
        for (int i = 0; i < tokens.length; i++)
        {
            if (i != 0)
                msg += "\\ ";
            msg += tokens[i];
        }
        return msg;
    }

    String unescape(String text)
    {
        String str = text.replace("\\\\", "\\");
        str = str.replace("\\ ", " ");
        return str;
    }


    void parseLobbyList(String[] parts)
    {
        if (inLobby.get())
            System.out.println("Players in lobby \""+lobbyName+"\":");
        else
            System.out.println("List of lobies:");
        for (int i = 1; i < parts.length; i++)
        {
            String name = unescape(parts[i]);
            System.out.println(name);
        }
    }
    void send(String msg)
    {
        try
        {
            out.write(msg.getBytes());
            if (DEBUG)
                System.out.print("SEND: "+msg);
        } catch (IOException e)
        {
            // No need to do anything endless recv cycle in main will handle reconnect
        }
    }

    public boolean haveCard(String card)
    {
        return false;
    }

    public void printCards()
    {
        for (Map.Entry<String,Integer> pair : cards.entrySet())
        {
            if (pair.getValue() > 0)
                System.out.println(pair.getKey() + " " + pair.getValue() + "x");
        }
    }

    public void requestLobbyList()
    {
        send("LOBBY_LIST"+DELIMITER);
    }

    public void createLobby(String name)
    {
        lobbyName = name;
        inLobby.set(true);
    }

    public void leaveLobby()
    {
        inLobby.set(false);
        lobbyName = "";
        System.out.println("You left lobby.");
        send("LOBBY_LIST"+DELIMITER);
    }

    public void joinLobby(String name)
    {
        lobbyName = name;
        inLobby.set(true);
        System.out.println("You joined lobby \""+name+"\"");
        send("LOBBY_LIST"+DELIMITER);
    }

}
