package bomberman;

import java.util.Scanner;

/**
 * Created by Štěpán Martínek on 24.12.2016.
 */
public class InputThread extends Thread
{
    Scanner scanner;
    Main game;


    public InputThread(Main main)
    {
        scanner = new Scanner(System.in);
        game = main;
    }

    public void run()
    {
        System.out.println("Use command 'controls' for list of commands.");
        cycle:
        while(scanner.hasNextLine() && game.running.get())
        {
            String line = scanner.nextLine();

            if (!game.running.get())
                break;

            String[] parts = line.split(" ");
            if (parts.length < 1)
                continue;

            if (!checkConnected(parts[0]))
            {
                System.out.println("Please wait attempting to reconnect...");
                continue;
            }

            switch (parts[0].toLowerCase())
            {
                case "debug":
                    game.DEBUG = !game.DEBUG;
                    break;
                case "lobby":
                {
                    game.requestLobbyList();
                    break;
                }
                case "create":
                {
                    String name = "";
                    for (int i = 1; i < parts.length; i++)
                        name += (i == 1 ? "" : " ") + parts[i];
                    game.send("CREATE_LOBBY " + game.escapeString(name) );
                    break;
                }
                case "leave":
                {
                    game.send("LEAVE_LOBBY" );
                    break;
                }
                case "join":
                {
                    String name = "";
                    for (int i = 1; i < parts.length; i++)
                        name += (i == 1 ? "" : " ") + parts[i];
                    game.send("JOIN_LOBBY " + game.escapeString(name) );
                    break;
                }
                case "cards":
                {
                    if (game.inGame.get())
                        game.send("CARDS");
                    else
                        System.out.println("You are not in a game.");
                    break;
                }
                case "play":
                {
                    if (game.inGame.get())
                        handlePlay(parts);
                    else
                        System.out.println("You are not in a game.");
                    break;
                }
                case "draw":
                {
                    if (game.inGame.get())
                        game.send("DRAW");
                    else
                        System.out.println("You are not in a game.");
                    break;
                }
                case "start":
                {
                    game.send("START_GAME");
                    break;
                }
                case "rules":
                    printRules();
                    break;
                case "controls":
                    printControls();
                    break;
                case "ff":
                    if (game.inGame.get())
                    {
                        game.send("FF" );
                    }
                    else
                        System.out.println("You are not in a game.");
                    break;
                case "quit":
                    game.send("QUIT");
                    game.running.set(false);
                    break cycle;
                case "kick":
                    game.send("KICK_PLAYER "+game.escapeString(parts[1]));
                    break;
                default:
                    System.out.println("Unknown command! Use command 'controls' for list of available commands.");
                    break;
            }
        }
        System.out.println("Bye from a thread!");
    }

    private boolean checkConnected(String part)
    {
        if (game.connected.get())
            return true;

        switch (part)
        {
            case "ff":
            case "lobby":
            case "play":
            case "start":
            case "cards":
            case "draw":
                return false;
        }

        return true;
    }

    private void handlePlay(String[] parts)
    {
        if (parts.length == 1)
        {
            System.out.println("End of turn!");
            return;
        }

        boolean have = false;
        if (parts[1].toLowerCase().equals("triple"))
            have = game.haveCard(parts[2].toUpperCase(),3);
        else
            have = game.haveCard(parts[1].toUpperCase(),1);

        if (!have)
        {
            System.out.println("You dont have this card!");
            return;
        }

        switch (parts[1].toLowerCase())
        {
            case "kick":
            case "run":
            case "peak":
            case "shuffle":
                game.send("PLAY "+parts[1].toUpperCase());
                break;
            case "triple":
            {
                if (parts.length < 4)
                {
                    System.out.println("Syntax for triple is: play triple <CARD> <NAME OF TARGET>");
                    break;
                }
                String s = "";
                for (int i = 3; i < parts.length; i++)
                    s += parts[i] + "";
                game.send("PLAY "+parts[1].toUpperCase() + " " +parts[2].toUpperCase()+" "+ game.escapeString(s) );
                break;
            }
            case "steal":
            {
                if (parts.length < 3)
                {
                    System.out.println("Syntax for steal is: play steal <NAME OF TARGET>");
                    break;
                }
                String s = "";
                for (int i = 2; i < parts.length; i++)
                    s += parts[i] + "";
                game.send("PLAY "+parts[1].toUpperCase() + " "+game.escapeString(s) );
                break;
            }
            default:
                System.out.println("Unknow play!");
                return;
        }
    }

    void printRules()
    {
        System.out.println("Rules:");
        System.out.println("Players taking turns. Each turn compose from two phases.");
        System.out.println("Phase 1: Player can play any number of cards.");
        System.out.println("Phase 2: Player draws a card, if card is not a bomb player take that card to his hand and his turn ends. If card is a bomb player lost.");
        System.out.println("Game ends when only one player is alive.");
    }

    void printControls()
    {
        System.out.println("Controls:");
        System.out.println("Lobby commands:");
        System.out.println("join <name of lobby> - Command for joining the lobby");
        System.out.println("create <name of lobby> - Command for creating the lobby");
        System.out.println("leave - Command to leave lobby");
        System.out.println("start - Start The Game");
        System.out.println("kick <name of player> - Command for kicking player from lobby");
        System.out.println("Game commands:");
        System.out.println("draw - Command for drawing a card (a possibly ending of turn)");
        System.out.println("cards - Command to get info about your cards and number of cards in pack and other player hands");
        System.out.println("ff - Command for forfeiting game, or leaving game after player went kaboom");
        System.out.println("play <CARD> - Play comand for cards KICK, RUN, PEAK and SHUFFLE");
        System.out.println("play <CARD> <Name of target player> - Play comand for cards STEAL");
        System.out.println("play triple <CARD> <Name of target player> - Play comand for TRIPLE");
        System.out.println("General commands:");
        System.out.println("rules - Print rules");
        System.out.println("controls - Print controls");
        System.out.println("quit - Exit from progam");
    }


}
