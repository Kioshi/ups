package bomberman;

import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.*;
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
    static final boolean DEBUG = true;
    Stage stage;
    Scene connectScene;
    Scene lobbyScene;
    Scene gameScene;
    Network network;


    public GUI(Stage primaryStage) throws IOException
    {
        network = new Network(this);
        stage = primaryStage;
        lobbyScene = createLobbyScene();
        gameScene = createGameScene();
        connectScene = createConnectScene();

    }

    private Scene createGameScene()
    {
        return null;
    }

    private Scene createLobbyScene()
    {
        BorderPane pane = new BorderPane();
        ListView<String> lv = new ListView<>();
        Button create = new Button("Create lobby");
        pane.setCenter(lv);
        pane.setBottom(create);

        return new Scene(pane);
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

            stage.setScene(createConnectingScene());
            try {
                network.connect(hostname,port,nick);
            } catch (IOException e) {
                e.printStackTrace();
            }

            //stage.setScene(lobbyScene);
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

    public void Connected()
    {
        stage.setScene(lobbyScene);

    }
}
