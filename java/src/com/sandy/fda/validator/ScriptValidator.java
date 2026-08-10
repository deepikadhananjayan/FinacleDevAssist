package com.sandy.fda.validator;

import java.util.List;

import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import com.sandy.fda.models.validator.Issue;
import com.sandy.fda.models.validator.Line;
import com.sandy.fda.models.validator.ScriptInfo;
import com.sandy.fda.models.validator.ScriptInfo.Builder;
import com.sandy.fda.parser.ScriptParser;
import com.sandy.fda.parser.TokenParser;
import com.sandy.fda.parser.Tokenizer;
import com.sandy.fda.validator.core.SimpleValidator;
import com.sandy.fda.validator.core.SyntaxValidator;

public class ScriptValidator {

    private ScriptParser scriptParser;
    private SimpleValidator simpleValidator;
    private SyntaxValidator syntaxValidator;

    public ScriptValidator(TokenParser tokenParser, Tokenizer tokenizer) {
        this.scriptParser = new ScriptParser(tokenParser);
        this.simpleValidator = new SimpleValidator();
        this.syntaxValidator = new SyntaxValidator(tokenParser, tokenizer);
    }

    public JsonObject validate(String filePath) throws Exception {

        List<Line> scrLines = scriptParser.parse(filePath, true);

        Builder builder = new ScriptInfo.Builder();
        builder.setAllLines(scrLines);
        ScriptInfo scriptInfo = builder.build();

        simpleValidator.validate(scriptInfo);
        syntaxValidator.validate(scriptInfo);

        return convertToJsonResponse(scriptInfo);
    }

    private JsonObject convertToJsonResponse(ScriptInfo scriptInfo) {

        JsonObject response = new JsonObject();
        JsonArray errors = new JsonArray();
        JsonArray warnings = new JsonArray();

        response.addProperty("STATUS", "SUCCESS");
        for (Issue issue : scriptInfo.getIssues()) {

            List<Line> lines = issue.getLines();
            int lineNo = (lines == null || lines.size() > 1) ? -1 : lines.get(0).getLineNo();
            String message = issue.getIssueMessage();

            JsonObject issueObj = new JsonObject();
            issueObj.addProperty("message", message);
            issueObj.addProperty("line", lineNo);

            switch (issue.getType()) {
                case ERROR:
                    errors.add(issueObj);
                    break;
                case WARNING:
                    warnings.add(issueObj);
                    break;
            }
        }

        response.add("errors", errors);
        response.add("warnings", warnings);
        response.addProperty("total_errors", errors.size());
        response.addProperty("total_warnings", warnings.size());

        return response;
    }
}
