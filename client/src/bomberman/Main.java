package bomberman;

import javafx.application.Application;
import javafx.stage.Stage;

import java.io.FileNotFoundException;
import java.io.IOException;

public class Main extends Application
{

    /**
     * Launch gui
     * @param args argument
     */
    public static void main(String[] args) throws FileNotFoundException
    {
        launch(args);
    }

    @Override
    public void start(Stage primaryStage) throws IOException
    {
        GUI gui = new GUI(primaryStage);

        primaryStage.setTitle("Bomberman");
        primaryStage.setScene(gui.connectScene);

        primaryStage.setMinHeight(300);
        primaryStage.setMinWidth(400);
        primaryStage.show();
    }

}