package bomberman;

import java.io.*;
import java.net.Socket;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Scanner;
import java.util.concurrent.atomic.AtomicBoolean;

import static java.lang.System.in;

public class Main
{
    static boolean DEBUG = false;
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
    String nick;
    PrintStream printStream;

    public static void main(String[] args) throws IOException
    {
        new Main(args);
    }

    Main(String[] args) throws FileNotFoundException
    {
        Scanner scanner = new Scanner(in);
        printStream = new PrintStream("network.log");
        connect(args,scanner);
        if (!login(scanner))
            return;

        connected.set(true);
        InputThread inputThread = new InputThread(this);
        inputThread.start();
        run();
    }

    void connect(String args[],Scanner scanner)
    {
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
                    if (args.length > 0)
                        host = args[0];
                    else
                    {
                        System.out.print("Addres: ");
                        host = scanner.nextLine();
                    }

                    if (args.length > 1)
                        port = Integer.parseInt(args[1]);
                    else
                    {
                        System.out.print("Port: ");
                        port = scanner.nextInt();
                        scanner.nextLine();
                    }
                    args = new String[0];
                    if (port >= 65536 || port == 0)
                    {
                        System.out.println("Wrong port");
                        continue;
                    }
                    clientSocket = new Socket(host, port);
                }
                br = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));

            }
            catch (Exception e)
            {
                System.out.println("Cant connect to server!");
                if (DEBUG)
                    System.exit(0);
            }

        }

    }

    boolean login(Scanner scanner)
    {
        System.out.print("Nickname: ");
        while(session == null)
        {
            String s;
            nick = scanner.nextLine().trim();
            if (nick.charAt(0) == '_' && nick.length() > 1)
            {
                s = "SESSION " + escapeString(new StringBuilder(nick.substring(1)).reverse().toString());
            }
            else
                s = "LOGIN " + escapeString(nick);

            try
            {
                out = clientSocket.getOutputStream();
                send(s);
                System.out.println("Logging...");
                session = br.readLine();
                if (DEBUG)
                    System.out.println("RECV: "+session);
                printStream.println("RECV: "+session);
            }
            catch (Exception e)
            {
                System.err.println("Server unexpectly closed connection!");
                return false;
            }

            if (session == null)
            {
                System.out.print("Nick is already taken, try another one: ");
                if (!reconnect())
                    return false;
            }
            else
            {
                String[] str = session.split(" ");
                if (str.length < 2 || !str[0].equals("SMSG_SESSION") || !nick.equals(new StringBuilder(str[1]).reverse().toString()))
                {
                    System.err.println("Server speaking gibberish!");
                    System.exit(0);
                }
                else
                {
                    session = "";
                    for (int i = 1; i < str.length; i++)
                    {
                        if (i!=1) session += " ";
                        session += str[i];
                    }
                }
            }
        }

        return true;
    }

    void run()
    {
        String msg = "";
        while (running.get())
        {
            try
            {
                msg = br.readLine();
            } catch (IOException e)
            {
                System.err.println("Server crashed!");
                running.set(false);
                System.exit(0);

            }
            if (msg == null && !attemptToReconnect())
            {
                if (running.get())
                    System.err.println("Cant reconnect!");
                running.set(false);
                System.exit(0);
            }

            if (msg == null)
                continue;

            if (DEBUG)
                System.out.println("RECV: "+msg);
            printStream.println("RECV: "+msg);

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
                String s;
                send("SESSION "+session);
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
            case "SMSG_LOBBY_LIST":
                parseLobbyList(parts);
                break;
            case "SMSG_CREATE_LOBBY":
                createLobby(parts[1]);
                break;
            case "SMSG_JOIN_LOBBY":
                joinLobby(parts[1]);
                break;
            case "SMSG_LEAVE_LOBBY":
                leaveLobby();
                break;
            case "SMSG_START_GAME":
                startGame(parts[1]);
                break;
            case "SMSG_MESSAGE":
                String s = "";
                for (int i = 1; i < parts.length; i++)
                    s += parts[i]+" ";
                System.out.println(s.replace("\\\\","\\").replace("\\ "," ").trim());
                break;
            case "SMSG_FF":
                ff();
                break;
            case "SMSG_DRAW":
                newCard(parts[1]);
                break;
            case "SMSG_DISCARD":
                removeCard(parts[1]);
                break;
            case "SMSG_CARDS":
                printCards(parts);
                break;
            case "SMSG_GAME_END":
                endGame(parts);
                break;
            case "SMSG_PLAY":
                play(parts);
                break;
            case "SMSG_TURN":
                System.out.println("Its your turn, this is what you can do:");
                canPlay();
                break;
        }

        return true;
    }

    private void play(String[] parts)
    {
        switch (parts[1].toLowerCase())
        {
            case "peak":
                System.out.println("You peaked top cards from deck (in this order):");
                for (int i = 2; i < parts.length; i++)
                    System.out.println(parts[i]);
                break;
            case "shuffle":
                System.out.println("You shuffled deck.");
                break;
            case "run":
                System.out.println("You ran from one bomb!");
                break;
            case "kick":
                System.out.println("You kicked one bomb to another player!");
                break;
            case "steal":
                System.out.println("You stole card from player.");
                break;
            case "triple":
                System.out.println("You discarded random card from player.");
                break;
        }
    }

    private void endGame(String[] parts)
    {
        inGame.set(false);
        if (parts.length == 1)
        {
            System.out.println("Game ended and somehow noone is a winner...");
        }
        else if (parts[1].equals(nick))
        {
            System.out.println("Congratulation you won The Game!");
        }
        else
            System.out.println("Player "+parts[1]+" win The Game!");
    }

    private void newCard(String card)
    {
        System.out.println("You have got new card: "+card);
        int count = cards.containsKey(card) ? cards.get(card) : 0;
        cards.put(card, count + 1);
    }

    private void removeCard(String card)
    {
        System.out.println("You lost card: "+card);
        int count = cards.containsKey(card) ? cards.get(card) : 0;
        if (count == 1)
            cards.remove(card);
        else if (count == 0)
        {

        }
        else
            cards.put(card,count -1);
    }

    private void ff()
    {
        System.out.println("You left the game.");
        inGame.set(false);
        send("LOBBY_LIST");
    }

    private void startGame(String name)
    {
        System.out.println("The Game has started. Starting player is "+name);
        inLobby.set(false);
        inGame.set(true);
        cards.clear();
        canPlay();
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
                messages.add(unescape(text.substring(0, i)));
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
            msg = "CMSG_"+msg+DELIMITER;
            out.write(msg.getBytes());
            if (DEBUG)
                System.out.print("SEND: "+msg);
            printStream.print("SEND: "+msg);
        } catch (IOException e)
        {
            // No need to do anything endless recv cycle in main will handle reconnect
        }
    }

    public boolean haveCard(String card, int count)
    {
        return cards.containsKey(card) && cards.get(card) >= count;
    }

    public void printCards(String[] parts)
    {
        for(int i = 1; i < parts.length;)
        {
            if (i == 1)
            {
                System.out.println("Number of cards in deck: "+parts[i]);
                i+=2;
                continue;
            }

            System.out.println("Player " + parts[i-1] + " have " + parts[i]+" cards");
            i+=2;
        }

        System.out.println("Your cards:");
        for (Map.Entry<String,Integer> pair : cards.entrySet())
        {
            if (pair.getValue() > 0)
                System.out.println(pair.getKey() + " " + pair.getValue() + "x");
        }
    }

    public void requestLobbyList()
    {
        send("LOBBY_LIST");
    }

    public void createLobby(String name)
    {
        lobbyName = name;
        inLobby.set(true);
        System.out.println("You created lobby \""+name+"\"");
    }

    public void leaveLobby()
    {
        inLobby.set(false);
        lobbyName = "";
        System.out.println("You left lobby.");
        send("LOBBY_LIST");
    }

    public void joinLobby(String name)
    {
        lobbyName = name;
        inLobby.set(true);
        System.out.println("You joined lobby \""+name+"\"");
        send("LOBBY_LIST");
    }
    
    void canPlay()
    {
        for(Map.Entry<String, Integer> entry : cards.entrySet())
        {
            switch (entry.getKey().toLowerCase())
            {

                case "kick":
                case "run":
                case "peak":
                case "shuffle":
                case "steal":
                    if (entry.getValue() >= 1)
                        System.out.println("You can play "+ entry.getKey());
                    break;
                default:
                    if (entry.getValue() >= 3)
                        System.out.println("You can play triple with "+ entry.getKey());
                    break;
            }
        }
        System.out.println("You can end turn by drawing a card.");
    }
}
