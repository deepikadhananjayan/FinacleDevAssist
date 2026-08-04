package com.sandy.fda.socket;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.BindException;
import java.net.ServerSocket;
import java.net.Socket;

import com.google.gson.Gson;
import com.google.gson.JsonObject;
import com.sandy.fda.beautifier.Beautifier;
import com.sandy.fda.parser.TokenParser;
import com.sandy.fda.parser.Tokenizer;
import com.sandy.fda.utils.FDAConstants;
import com.sandy.fda.utils.FDALogger;
import com.sandy.fda.validator.ScriptValidator;

public class FDASocket {

    private TokenParser tokenParser;
    private Tokenizer tokenizer;
    private ScriptValidator scriptValidator;
    private Beautifier beautifier;

    public FDASocket() {
        this.tokenParser = new TokenParser();
        this.tokenizer = new Tokenizer(tokenParser);
        this.scriptValidator = new ScriptValidator(tokenParser, tokenizer);
        this.beautifier = new Beautifier(tokenParser, tokenizer);
    }

    public void initSocket() {

        ServerSocket server = null;

        try {

            try {
                server = new ServerSocket(FDAConstants.getPort());
            } catch (BindException e) {
                FDALogger.info(
                        "Port [" + FDAConstants.getPort() + "] unavailable, using dynamic port (Letting OS Decide)");

                server = new ServerSocket(0);
                FDAConstants.updatePortInProperties(server.getLocalPort());
            }

            FDALogger.info("Waiting for plugin connection...");

            Socket client = server.accept();

            FDALogger.info("Plugin connected");

            handleRequests(client);

        } catch (Exception e) {
            FDALogger.error(e);
        } finally {
            try {
                if (server != null && !server.isClosed()) {
                    server.close();
                }
            } catch (Exception e) {
                FDALogger.error(e);
            }
            FDALogger.info("Server closed");
        }
    }

    private void handleRequests(Socket client) {

        try (
                BufferedReader in = new BufferedReader(
                        new InputStreamReader(client.getInputStream()));

                PrintWriter out = new PrintWriter(
                        client.getOutputStream(),
                        true)) {

            Gson gson = new Gson();

            boolean shutdown = false;

            String raw;

            while (!shutdown && (raw = in.readLine()) != null) {

                FDALogger.info("Received: " + raw);

                JsonObject req = gson.fromJson(raw, JsonObject.class);

                String type = req.get("type").getAsString();

                JsonObject response = new JsonObject();

                try {
                    switch (type) {

                        case "GET_KEYWORDS_AND_USERHOOKS":
                            response = tokenParser.getKeywordsAndUserhooks();
                            break;

                        case "VALIDATE_SCRIPT":
                            String filePath = req.get("filePath").getAsString();
                            response = scriptValidator.validate(filePath);
                            break;

                        case "BEAUTIFY_CODE":
                            filePath = req.get("filePath").getAsString();
                            response = beautifier.beautifyCode(filePath);
                            break;

                        case "GET_SUGGESTIONS":
                            // response = handleSuggest(req);
                            break;

                        case "SHUTDOWN":
                            response.addProperty("STATUS", "SUCCESS");
                            shutdown = true;
                            break;

                        default:
                            FDALogger.info("Unknown request: " + type);
                            response.addProperty("STATUS", "FAILED");
                            response.addProperty("message", "Unknown Request Type!");
                            break;
                    }
                } catch (Exception e) {
                    FDALogger.error(e);
                    e.printStackTrace();
                    String excpMsg = e.getClass().getSimpleName()
                            + " : "
                            + (e.getMessage() != null ? e.getMessage() : "No message!")
                            + " while handling request type : "
                            + type;
                    response.addProperty("STATUS", "EXCEPTION");
                    response.addProperty("EXCEPTION", excpMsg);
                }

                out.println(response.toString());
                out.flush();

                FDALogger.info("Response sent");
            }

        } catch (Exception e) {
            FDALogger.error(e);
        } finally {
            try {
                client.close();
            } catch (Exception e) {
                FDALogger.error(e);
            }
            FDALogger.info("Client disconnected");
        }
    }
}
