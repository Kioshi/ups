package bomberman;

import java.util.Scanner;

import static bomberman.Main.DELIMITER;

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
        System.out.println("Hello from a thread!");
        cycle:
        while(scanner.hasNextLine() && game.running.get())
        {
            String line = scanner.nextLine();

            String[] parts = line.split(" ");
            if (parts.length < 1)
                continue;

            if (!checkConnected(parts[0]))
            {
                System.out.println("Please wait attempting to reconnect...");
                continue;
            }

            switch (parts[0])
            {
                case "lobby":
                {
                    parseLobby(parts);
                    break;
                }
                case "cards":
                {
                    game.printCards();
                    break;
                }
                case "play":
                {
                    handlePlay(parts);
                    break;
                }
                case "rules":
                    printRules();
                    break;
                case "controls":
                    printControls();
                    break;
                case "ff":
                    System.out.println("You forfeited match!");
                    game.send("FF"+DELIMITER);
                    break;
                case "quit":
                    game.running.set(false);
                    break cycle;
                case "kick":
                    game.send("KICK_PLAYER "+game.escapeString(parts[1])+DELIMITER);
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
                return false;
        }

        return true;
    }

    private void parseLobby(String[] parts)
    {
        if (parts.length == 1)
        {
            game.requestLobbyList();
            return;
        }

        String name = "";
        for (int i = 2; i < parts.length; i++)
            name+= (i == 2 ? "": " ") + parts[i];
        String s = null;
        switch (parts[1])
        {
            case "create":
                s = "CREATE_LOBBY " + game.escapeString(name) + DELIMITER;
                break;
            case "leave":
                s = "LEAVE_LOBBY" + DELIMITER;
                break;
            case "join":
                s = "JOIN_LOBBY " + game.escapeString(name) + DELIMITER;
                break;
            default:
                System.out.println("Unknown lobby subcommand. Use 'controls' command for available commands.");
                break;
        }
        if (s != null)
        {
            game.send(s);
        }
    }

    private void printControls()
    {

    }

    private void handlePlay(String[] parts)
    {
        if (parts.length == 1)
        {
            System.out.println("End of turn!");
            return;
        }

        if (!game.haveCard(parts[1]))
        {
            System.out.println("You dont have this card!");
            return;
        }

        switch (parts[1])
        {
            case "double":
                break;
            case "triple":
                break;
            case "kick":
                break;
            case "run":
                break;
            case "peak":
                break;
            case "steal":
                break;
            case "shuffle":
                break;
            default:
                System.out.println("Unknow play!");
                return;
        }
    }

    private void printRules()
    {
        System.out.println("Players taking turns. Each turn compose from two phases.");
        System.out.println("Phase 1: Player can play any number of cards.");
        System.out.println("Phase 2: Player draws a card, if card is not a bomb player take that card to his hand and his turn ends.");
        System.out.println("If card is a bomb and player doesnt have defuse card, he lost.");
        System.out.println("If he have defuse card he have, card is automaticly used and player can put card anywhere in pack.");
        System.out.println("Game ends when only one player is alive.");
    }
}
