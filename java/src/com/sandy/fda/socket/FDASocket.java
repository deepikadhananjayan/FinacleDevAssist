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
import com.sandy.fda.custom24.Custom24Handler;
import com.sandy.fda.fi.FinacleInterfaceHandler;
import com.sandy.fda.parser.TokenParser;
import com.sandy.fda.parser.Tokenizer;
import com.sandy.fda.utils.FDAConstants;
import com.sandy.fda.utils.FDALogger;
import com.sandy.fda.validator.ScriptValidator;

public class FDASocket {

    private TokenParser tokenParser;
    private Custom24Handler custom24Handler;
    private Tokenizer tokenizer;
    private ScriptValidator scriptValidator;
    private Beautifier beautifier;
    private FinacleInterfaceHandler finacleInterfaceHandler;

    public FDASocket() {
        this.tokenParser = new TokenParser();
        this.custom24Handler = new Custom24Handler();
        this.tokenizer = new Tokenizer(tokenParser);
        this.scriptValidator = new ScriptValidator(tokenParser, tokenizer);
        this.beautifier = new Beautifier(tokenParser, tokenizer);
        try {
            this.finacleInterfaceHandler = new FinacleInterfaceHandler(beautifier);
        } catch (Exception e) {
            FDALogger.error(e);
        }
    }

    public void initSocket() {

        ServerSocket server = null;

        try {
            int port = Integer.parseInt(FDAConstants.getProperties().get("java.port"));
            try {
                server = new ServerSocket(port);
                if (port == 0) {
                    FDALogger.info(
                            "User Assigned [" + port + "] in Properties, using dynamic port (Letting OS Decide)");
                    FDAConstants.updatePortInProperties(server.getLocalPort());
                }
            } catch (BindException e) {
                FDALogger.info(
                        "Port [" + port + "] unavailable, using dynamic port (Letting OS Decide)");

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

                JsonObject req = gson.fromJson(raw, JsonObject.class);

                String type = req.remove("type").getAsString();

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
                            response = beautifier.beautifyCode(req);
                            break;

                        case "EXECUTE_FI_REQUEST":
                            if (FDAConstants.getOldPropFileSize() != FDAConstants.getPropertiesFileSize()) {
                                if (!FDAConstants.load()) {
                                    throw new IllegalStateException("Failed to Update Properties");
                                }
                            }
                            response = finacleInterfaceHandler.execute(req);
                            break;

                        case "GENERATE_CUSTOM_MENU":
                            response = custom24Handler.generateCustomMenu(req);
                            break;
                        
                        case "DEPLOY_CUSTOM_MENU":
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

                FDALogger.info(response.toString());

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
