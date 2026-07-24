package com.sandy.fda.parser;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import com.sandy.fda.models.Issue;
import com.sandy.fda.models.Line;
import com.sandy.fda.models.SubToken;
import com.sandy.fda.models.Token;
import com.sandy.fda.models.enums.IssueType;
import com.sandy.fda.models.enums.TokenType;
import com.sandy.fda.utils.FDAIssueMessage;
import com.sandy.fda.utils.ValidatorUtil;

public class Tokenizer {

    private TokenParser tokenParser;
    private Map<String,Token> knownFunctions = null;

    public Tokenizer(TokenParser tokenParser) {
        this.tokenParser = tokenParser;
    }

    public Object tokenize(Line line) throws Exception{

        String lineData = ValidatorUtil.rmvCmtNdTrlgSpc(line.getLineContent());
        List<SubToken> tokens = new ArrayList<>();

        int i = 0;
        while (i < lineData.length()) {
            char c = lineData.charAt(i);

            if (Character.isWhitespace(c)) {
                i++;
                continue;
            }

            if (c == '(') {
                tokens.add(
                        new SubToken(
                                TokenType.OPEN_BRACKET,
                                "("));
                i++;
                continue;
            }

            if (c == ')') {
                tokens.add(
                        new SubToken(
                                TokenType.CLOSE_BRACKET,
                                ")"));
                i++;
                continue;
            }

            if (c == '"' || c == '\'') {
                char quote = c;
                StringBuilder value = new StringBuilder();
                value.append(c);
                i++;
                while (i < lineData.length()
                        && lineData.charAt(i) != quote) {
                    value.append(lineData.charAt(i));
                    i++;
                }
                if (i >= lineData.length()) {
                    return new Issue.Builder()
                            .addLine(line)
                            .setType(IssueType.ERROR)
                            .setIssueMessage(FDAIssueMessage.UNCLOSED_STRING_OR_CHAR_LITERAL_IN_LINE +
                                    line.getLineNo());
                }

                value.append(lineData.charAt(i));
                i++;
                tokens.add(
                        new SubToken(
                                TokenType.STRING_OR_CHAR_LITERAL,
                                value.toString()));
                continue;
            }

            if (Character.isDigit(c)) {
                StringBuilder value = new StringBuilder();
                while (i < lineData.length()
                        && Character.isDigit(lineData.charAt(i))) {
                    value.append(lineData.charAt(i));
                    i++;
                }
                tokens.add(
                        new SubToken(
                                TokenType.NUMBER_LITERAL,
                                value.toString()));
                continue;
            }

            if (c == '=' || c == '!' || c == '>' || c == '<') {

                StringBuilder op = new StringBuilder();
                op.append(c);
                i++;

                if (i < lineData.length()) {

                    char next = lineData.charAt(i);

                    if (next == '=') {
                        op.append(next);
                        i++;
                    } else if (next == '>' || next == '<' || next == '!') {
                        return new Issue.Builder()
                                .addLine(line)
                                .setType(IssueType.ERROR)
                                .setIssueMessage("(" + c + next + ")" +
                                        FDAIssueMessage.INVALID_OPERATOR_SEQUENCE_IN_LINE + line.getLineNo());
                    }
                }

                String operator = op.toString();

                if (!tokenParser.COMPARISON_OPERATORS.contains(operator)) {
                    return new Issue.Builder()
                            .addLine(line)
                            .setType(IssueType.ERROR)
                            .setIssueMessage("(" + operator + ") " +
                                    FDAIssueMessage.INVALID_OPERATOR_SEQUENCE_IN_LINE +
                                    line.getLineNo());
                }

                tokens.add(
                        new SubToken(
                                TokenType.COMPARISON_OPERATOR,
                                operator));

                continue;
            }

            if (c == ',') {
                tokens.add(
                        new SubToken(
                                TokenType.COMMA,
                                ","));
                i++;
                continue;
            }

            if (Character.isLetter(c) || c == '_') {
                StringBuilder word = new StringBuilder();
                while (i < lineData.length()
                        &&
                        (Character.isLetterOrDigit(lineData.charAt(i))
                                || lineData.charAt(i) == '.'
                                || lineData.charAt(i) == '$'
                                || lineData.charAt(i) == '_')) {
                    word.append(lineData.charAt(i));
                    i++;
                }
                String value = word.toString();
                if (value.equals("AND")) {
                    tokens.add(
                            new SubToken(
                                    TokenType.AND_OPERATOR,
                                    value));
                } else if (value.equals("OR")) {
                    tokens.add(
                            new SubToken(
                                    TokenType.OR_OPERATOR,
                                    value));
                } else if (isBuiltInFunction(value)) {
                    tokens.add(
                            new SubToken(
                                    TokenType.FUNCTION,
                                    value));

                } else if (value.indexOf(".") != -1) {
                    tokens.add(
                            new SubToken(
                                    TokenType.REP_VARIABLE,
                                    value));
                } else if (value.startsWith("sv_")) {
                    tokens.add(
                            new SubToken(
                                    TokenType.SV_VARIABLE,
                                    value));
                } else if (value.startsWith("fv_")) {
                    tokens.add(new SubToken(TokenType.FV_VARIABLE,
                                            value));
                }
                continue;
            }

            if (c == '+' || c == '-' || c == '*' || c == '/') {
                tokens.add(new SubToken(
                        TokenType.ARITHMETIC_OPERATOR, String.valueOf(c)));
                i++;
                continue;
            }

            return new Issue.Builder()
                    .addLine(line)
                    .setType(IssueType.ERROR)
                    .setIssueMessage(FDAIssueMessage.UNEXPECTED_TOKEN_IN_LINE + line.getLineNo())
                    .build();
        }

        return tokens;
    }

    private boolean isBuiltInFunction(String function) throws Exception{
        
        if (knownFunctions == null) {
            this.knownFunctions = tokenParser.getFunctionMap();
        }

        String lcFunction = function.toLowerCase();

        for (String key : knownFunctions.keySet()) {
            String lcKey = getFunctionWthotBrcks(key);
            
            if (lcFunction.equals(lcKey))
                return true;
        }
        return false;
    }

    private String getFunctionWthotBrcks(String func) {
        int endIdx = func.toLowerCase().indexOf("(");
        return func.substring(0, endIdx).toLowerCase();
    }
}
