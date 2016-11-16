package bomberman;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.Scanner;

import static bomberman.GUI.DEBUG;
/**
 * Created by Štěpán Martínek on 15.11.2016.
 */
public class Network
{
    GUI gui;
    String nick;
    String session;

    Network(GUI _gui)
    {
        gui = _gui;
    }

    public void connect(String hostname, int port, String login) throws IOException {

        Socket clientSocket;
        if (DEBUG)
            clientSocket = new Socket("localhost",10001);
        else
        {
            clientSocket = new Socket(hostname, port);
        }

        InputStream in = clientSocket.getInputStream();
        OutputStream out = clientSocket.getOutputStream();

        nick = login;
        System.out.println("Login: "+login);
        send(out,new Packet(bomberman.Packet.LOGIN, login.length(),login.getBytes()));
        Packet packet = recv(in);
        String session = new String(packet.data);
        System.out.println("Session: "+session);
        this.session = session;
        gui.Connected();
    }

    private static Packet recv(InputStream in) throws IOException
    {
        byte[] opcode = new byte[1];
        in.read(opcode);
        if (true)
        {
            byte[] size = new byte[1];
            in.read(size);
            byte[] data = new byte[size[0]];
            in.read(data,0,size[0]);
            return new Packet(opcode[0],size[0],data);
        }
        return new Packet(opcode[0]);
    }

    private static void send(OutputStream out, Packet packet) throws IOException
    {
        out.write(packet.opcode);
        if (packet.size > 0)
        {
            out.write(packet.size);
            out.write(packet.data);
        }
    }
}
