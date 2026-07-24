package com.sandy.fda.validator.line.rules;

import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

import com.sandy.fda.models.Issue;
import com.sandy.fda.models.Line;
import com.sandy.fda.models.ScriptInfo;
import com.sandy.fda.models.SubToken;
import com.sandy.fda.models.Token;
import com.sandy.fda.models.enums.IssueType;
import com.sandy.fda.models.enums.TokenType;
import com.sandy.fda.parser.TokenParser;
import com.sandy.fda.utils.FDAIssueMessage;

public class FunctionValidator {

    private TokenParser tokenParser;
    private Map<String,Token> knownFunctions = null;

    public FunctionValidator(TokenParser tokenParser){
        this.tokenParser = tokenParser;
    }

    public void validate(Line line, List<SubToken> tokens, ScriptInfo scriptInfo){
        
    }

    public int validateFunction(int idx, List<SubToken> tokens, StringBuilder function,
            Line line, ScriptInfo scriptInfo) throws Exception {

        if (knownFunctions == null) {
            this.knownFunctions = tokenParser.getFunctionMap();
        }

        int start = idx;

        String functionName = tokens.get(idx).getValue();

        if ((idx + 1) >= tokens.size()
                || tokens.get(idx + 1).getType() != TokenType.OPEN_BRACKET) {

            scriptInfo.getIssues().add(new Issue.Builder()
                    .addLine(line)
                    .setIssueMessage(
                            FDAIssueMessage.MISSING_CLOSING_BRACKET_IN_FUNCTION + functionName + " in "
                                    + line.getLineNo())
                    .setType(IssueType.ERROR)
                    .build());

            return -1;
        }

        int level = 0;

        while (idx < tokens.size()) {

            if (tokens.get(idx).getType() == TokenType.OPEN_BRACKET)
                level++;

            if (tokens.get(idx).getType() == TokenType.CLOSE_BRACKET) {

                level--;

                if (level == 0)
                    break;
            }

            idx++;
        }

        if (idx >= tokens.size()) {
            scriptInfo.getIssues().add(new Issue.Builder()
                    .addLine(line)
                    .setIssueMessage(
                            FDAIssueMessage.MISSING_CLOSING_BRACKET_IN_FUNCTION + functionName + " in "
                                    + line.getLineNo())
                    .setType(IssueType.ERROR)
                    .build());
            return -1;
        }

        List<SubToken> functionTokens = tokens.subList(start, idx + 1);

        function.append(functionTokens.stream()
                .map(SubToken::getValue)
                .collect(Collectors.joining()));

        Issue issue = inspectFunction(functionTokens, line);

        if (issue != null) {
            scriptInfo.getIssues().add(issue);
            return -1;
        }

        return idx;
    }

    private Issue inspectFunction(List<SubToken> tokens, Line line) throws Exception {

        String functionName = tokens.get(0).getValue();

        if (tokens.get(1).getType() != TokenType.OPEN_BRACKET) {
            return new Issue.Builder()
                    .addLine(line)
                    .setIssueMessage(FDAIssueMessage.MISSING_OPENING_BRACKET_IN_FUNCTION + functionName + " in "
                            + line.getLineNo())
                    .setType(IssueType.ERROR)
                    .build();
        }

        Token function = null;

        for (String key : knownFunctions.keySet()) {
            String lcKey = getFunctionWthotBrcks(key);
            String lcFunction = functionName.toLowerCase();
            if (lcFunction.equals(lcKey)) {
                function = knownFunctions.get(key);
                break;
            }
        }

        if (function == null) {
            return new Issue.Builder()
                    .addLine(line)
                    .setIssueMessage(
                            "[" + functionName + "] " + FDAIssueMessage.UNKNOWN_FUNCTION_CALLED + line.getLineNo())
                    .setType(IssueType.WARNING)
                    .build();
        }

        int expectedParams = function.getParamInputs().size();

        int params = 0;
        int level = 0;
        boolean paramStarted = false;

        for (int i = 1; i < tokens.size(); i++) {

            SubToken token = tokens.get(i);

            switch (token.getType()) {

                case OPEN_BRACKET:
                    level++;
                    break;

                case CLOSE_BRACKET:

                    if (paramStarted && level == 1) {
                        params++;
                        paramStarted = false;
                    }

                    level--;

                    break;

                case COMMA:

                    if (level == 1) {

                        if (!paramStarted) {
                            return new Issue.Builder()
                                    .addLine(line)
                                    .setIssueMessage("[" + functionName + "] "
                                            + FDAIssueMessage.EMPTY_PARAMETERS_FOR_FUNCTION + line.getLineNo())
                                    .setType(IssueType.WARNING)
                                    .build();
                        }

                        params++;
                        paramStarted = false;
                    }

                    break;

                case FUNCTION:

                    int nested = i;
                    int nestedLevel = 0;

                    while (nested < tokens.size()) {

                        if (tokens.get(nested).getType() == TokenType.OPEN_BRACKET)
                            nestedLevel++;

                        if (tokens.get(nested).getType() == TokenType.CLOSE_BRACKET) {

                            nestedLevel--;

                            if (nestedLevel == 0)
                                break;
                        }

                        nested++;
                    }

                    if (nested >= tokens.size())
                        return new Issue.Builder()
                                .addLine(line)
                                .setIssueMessage(
                                        FDAIssueMessage.MISSING_CLOSING_BRACKET_IN_FUNCTION + functionName + " in "
                                                + line.getLineNo())
                                .setType(IssueType.ERROR)
                                .build();

                    List<SubToken> nestedTokens = tokens.subList(i, nested + 1);

                    Issue issue = inspectFunction(nestedTokens, line);

                    if (issue != null)
                        return issue;

                    paramStarted = true;

                    i = nested;

                    break;

                case REP_VARIABLE:
                case SV_VARIABLE:
                case FV_VARIABLE:
                case STRING_OR_CHAR_LITERAL:
                case NUMBER_LITERAL:

                    paramStarted = true;
                    break;

                case ARITHMETIC_OPERATOR:
                case COMPARISON_OPERATOR:
                    break;

                default:
                    return new Issue.Builder()
                            .addLine(line)
                            .setIssueMessage("[" + functionName + "] contains [" + token.getValue() + "]" +
                                    FDAIssueMessage.UNEXPECTED_TOKEN_IN_LINE + line.getLineNo())
                            .setType(IssueType.ERROR)
                            .build();
            }
        }

        if (params != expectedParams)
            return new Issue.Builder()
                    .addLine(line)
                    .setIssueMessage(
                            FDAIssueMessage.PARAMETER_COUNT_MISMATCH_FOR_FUNCTION + "[" + functionName + "] expected : "
                                    + expectedParams + ", found : " + params + " in " + line.getLineNo())
                    .setType(IssueType.ERROR)
                    .build();

        return null;
    }

     private String getFunctionWthotBrcks(String func) {
        int endIdx = func.toLowerCase().indexOf("(");
        return func.substring(0, endIdx).toLowerCase();
    }
}
