package com.sandy.fda.beautifier;

import com.google.gson.JsonObject;
import com.sandy.fda.parser.TokenParser;
import com.sandy.fda.parser.Tokenizer;

public class Beautifier {

    private BeautifierFactory factory;

    public Beautifier(TokenParser tokenParser, Tokenizer tokenizer) {
        this.factory = new BeautifierFactory(tokenParser, tokenizer);
    }

    public JsonObject beautifyCode(String filePath) throws Exception {
        ICodeBeautifier formatter = factory.getFormatterForFile(filePath);
        String beautifiedCode = formatter.beautify(filePath);
        return convertToJsonObject(beautifiedCode);
    }

    private JsonObject convertToJsonObject(String beautifiedCode) {
        JsonObject response = new JsonObject();
        response.addProperty("STATUS", "SUCCESS");
        response.addProperty("beautifiedCode", beautifiedCode);
        return response;
    }
}
