package bomberman;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.Arrays;
import java.util.Scanner;

import static bomberman.Packet.LOGIN;

public class Main
{
    static boolean DEBUG = true;
    public static void main(String[] args) throws IOException
    {
        Scanner scanner = new Scanner(System.in);

        Socket clientSocket;
        if (DEBUG)
            clientSocket = new Socket("localhost",10002);
        else
        {
            System.out.print("Addres: ");
            String host = scanner.next();

            System.out.print("Port: ");
            int port = scanner.nextInt();
            if (port >= 65536 || port == 0)
            {
                System.out.println("Wrong port");
                return;
            }
            clientSocket = new Socket(host, port);
        }

        InputStream in = clientSocket.getInputStream();
        OutputStream out = clientSocket.getOutputStream();

        System.out.print("Login: ");
        String login = scanner.next();
        send(out,new Packet(LOGIN,login.length(),login.getBytes()));
        Packet packet = recv(in);
        String session = new String(packet.data);
        System.out.println(session);
        scanner.next();

       /* while (scanner.hasNextLine())
        {
            String out = scanner.nextLine();
            clientSocket.getOutputStream().write(out.getBytes());

            byte[] buffer = new byte[500];
            clientSocket.getInputStream().read(buffer);
            String s = new String(buffer);
            System.out.println(new String(buffer));
        }*/
        clientSocket.close();
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



class Packet
{
    static byte LOGIN = 0;
    static byte SESSION = 1;

    Packet(byte _opcode, int _size, byte[] _data)
    {
        opcode = _opcode;
        size = (byte)_size;
        if (_data != null)
            data = Arrays.copyOf(_data, _data.length);
    }

    Packet(byte _opcode)
    {
        this(_opcode,0,null);
    }

    byte getSize()
    {
        return (byte)(1 + (size > 0 ? 1 : 0) + size);
    }

    byte opcode;
    byte size;
    byte data[];
};
