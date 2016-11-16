package bomberman;

import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.Spinner;
import javafx.scene.control.TextField;
import javafx.scene.layout.*;
import javafx.scene.paint.Color;
import javafx.stage.Stage;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;


/**
 * Created by Štěpán Martínek on 15.11.2016.
 */
public class GUI
{
    private static final boolean DEBUG = true;
    Stage stage;
    Scene connectScene;
    Scene lobbyScene;
    Scene gameScene;
    Network network = new Network();


    public GUI(Stage primaryStage) throws IOException
    {
        stage = primaryStage;
        connectScene = createConnectScene();
        lobbyScene = createLobbyScene();
        gameScene = createGameScene();
/*
        Scanner scanner = new Scanner(System.in);

        Socket clientSocket;
        if (DEBUG)
            clientSocket = new Socket("localhost",10001);
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
        send(out,new Packet(bomberman.Packet.LOGIN, login.length(),login.getBytes()));
        Packet packet = recv(in);
        String session = new String(packet.data);
        System.out.println(session);
        scanner.next();

        clientSocket.close();
*/
    }

    private Scene createGameScene()
    {
        return null;
    }

    private Scene createLobbyScene()
    {
        return null;
    }

    private Scene createConnectScene()
    {
        VBox vBox = new VBox();
        HBox nickBox = new HBox();
        Label nickL = new Label("Nick:");
        TextField nickTF = new TextField();
        nickBox.getChildren().addAll(nickL,nickTF);
        nickBox.setAlignment(Pos.BOTTOM_CENTER);
        nickBox.setSpacing(10.0);
        Label hostLabel = new Label("Hostname:");
        TextField hostTF = new TextField();
        HBox hostBox = new HBox();
        hostBox.getChildren().addAll(hostLabel,hostTF);
        hostBox.setSpacing(10.0);
        hostBox.setAlignment(Pos.BOTTOM_CENTER);
        HBox portBox = new HBox();
        Label portLabel = new Label("Port:");
        Spinner<Integer> portS = new Spinner<>(1,65536,1);
        portS.setEditable(true);
        portBox.getChildren().addAll(portLabel,portS);
        portBox.setSpacing(10.0);
        portBox.setAlignment(Pos.BOTTOM_CENTER);
        Button button = new Button("Connect");
        button.setAlignment(Pos.BOTTOM_CENTER);
        vBox.getChildren().addAll(nickBox,hostBox,portBox,button);
        vBox.setSpacing(10.0);


        button.setOnAction(event ->
        {
            String hostname = hostTF.getText();
            String nick = nickTF.getText();
            int port = portS.getValue();
            if (hostname.isEmpty())
                hostTF.setBorder(new Border(new BorderStroke(Color.RED, BorderStrokeStyle.SOLID, CornerRadii.EMPTY, BorderWidths.DEFAULT)));
            if (nick.isEmpty())
                nickTF.setBorder(new Border(new BorderStroke(Color.RED, BorderStrokeStyle.SOLID, CornerRadii.EMPTY, BorderWidths.DEFAULT)));
            if (nick.isEmpty() || hostname.isEmpty())
                return;

            network.connect(hostname,port,nick);
            stage.setScene(createConnectingScene());
        });
        hostTF.setOnKeyTyped(event -> hostTF.setBorder(null));
        nickTF.setOnKeyTyped(event -> nickTF.setBorder(null));
        nickTF.textProperty().addListener((ov, oldValue, newValue) ->
        {
            if (nickTF.getText().length() > 20)
                nickTF.setText(nickTF.getText().substring(0, 20));
        });

        Scene scene = new Scene(vBox);
        return scene;
    }

    private Scene createConnectingScene()
    {
        return new Scene(new Label("Connectiong..."));
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
