package com.sandy.fda.validator;

import java.util.List;

import com.google.gson.JsonObject;
import com.sandy.fda.models.Issue;
import com.sandy.fda.models.Line;
import com.sandy.fda.models.ScriptInfo;
import com.sandy.fda.models.ScriptInfo.Builder;
import com.sandy.fda.parser.ScriptParser;
import com.sandy.fda.parser.TokenParser;
import com.sandy.fda.validator.core.SimpleValidator;
import com.sandy.fda.validator.core.SyntaxValidator;

public class ScriptValidator {

    private static ScriptParser scriptParser;
    private static SimpleValidator simpleValidator;
    private static SyntaxValidator syntaxValidator;

    public ScriptValidator(TokenParser tokenParser){
        scriptParser = new ScriptParser();
        simpleValidator = new SimpleValidator();
        syntaxValidator = new SyntaxValidator(tokenParser);
    }

    public JsonObject validate(String filePath) throws Exception {

        List<Line> scrLines = scriptParser.parse(filePath);

        Builder builder = new ScriptInfo.Builder();
        builder.setAllLines(scrLines);
        ScriptInfo scriptInfo = builder.build();

        simpleValidator.validate(scriptInfo);
        syntaxValidator.validate(scriptInfo);

        return convertToJsonResponse(scriptInfo);
    }

    private JsonObject convertToJsonResponse(ScriptInfo scriptInfo) {
        
        JsonObject response = new JsonObject();

        response.addProperty("STATUS", "SUCCESS");

        for (Issue issue : scriptInfo.getIssues()) {
            System.out.println(issue);
        }

        return response;
    }
}
