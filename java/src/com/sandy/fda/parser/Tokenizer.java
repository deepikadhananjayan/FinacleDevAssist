package com.sandy.fda.parser;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import com.sandy.fda.models.validator.Issue;
import com.sandy.fda.models.validator.Line;
import com.sandy.fda.models.validator.SubToken;
import com.sandy.fda.models.validator.Token;
import com.sandy.fda.models.validator.enums.IssueType;
import com.sandy.fda.models.validator.enums.TokenType;
import com.sandy.fda.utils.FDAIssueMessage;

import com.sandy.fda.utils.FDAUtils;

public class Tokenizer {

    private TokenParser tokenParser;
    private Map<String, Token> knownFunctions = null;

    private final String START = "<--START";
    private final String END = "END-->";
    private final String TRACE_ON = "TRACE ON";
    private final String TRACE_OFF = "TRACE OFF";
    private final String AND = "AND";
    private final String OR = "OR";
    private final String LIBNAME = "LIBNAME";
    private final String IMPORT = "IMPORT";
    private final String GOTO = "GOTO";
    private final String GOSUB = "GOSUB";
    private final String IF = "IF";
    private final String ELSE = "ELSE";
    private final String THEN = "THEN";
    private final String ENDIF = "ENDIF";
    private final String WHILE = "WHILE";
    private final String DO = "DO";
    private final String EXITSCRIPT = "EXITSCRIPT";

    public Tokenizer(TokenParser tokenParser) {
        this.tokenParser = tokenParser;
    }

    public Object tokenize(Line line) throws Exception {

        String lineData = line.getLineContent().trim();
        List<SubToken> tokens = new ArrayList<>();

        int i = 0;
        while (i < lineData.length()) {
            char c = lineData.charAt(i);

            if (Character.isWhitespace(c)) {
                i++;
                continue;
            }

            if (c == '#') {
                StringBuilder value = new StringBuilder();
                while (i < lineData.length()) {
                    value.append(lineData.charAt(i));
                    i++;
                }

                tokens.add(
                        new SubToken(
                                TokenType.COMMENT_CONTENT,
                                value.toString()));
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

                    TokenType tokenType = tokens.size() > 0 ? tokens.get(0).getType() : null;

                    if (next == '=') {
                        op.append(next);
                        i++;
                    } else if (next == '>' || next == '<' || next == '!') {
                        return new Issue.Builder()
                                .addLine(line)
                                .setType(IssueType.ERROR)
                                .setIssueMessage("[" + c + next + "] " +
                                        FDAIssueMessage.INVALID_OPERATOR_SEQUENCE_IN_LINE + line.getLineNo())
                                .build();
                    } else if (c == '<' && next == '-') {
                        op.append(next);
                        i++;
                        while (i < lineData.length() && START.startsWith(op.toString())) {
                            op.append(lineData.charAt(i));
                            i++;
                        }
                        if (op.toString().equalsIgnoreCase(START)) {
                            tokens.add(
                                    new SubToken(
                                            TokenType.START,
                                            op.toString()));
                            continue;
                        } else {
                            return FDAUtils.buildUnexpectedTokenIssue(line);
                        }
                    } else if (tokenType == TokenType.REP_VARIABLE
                            || tokenType == TokenType.SV_VARIABLE
                            || tokenType == TokenType.FV_VARIABLE
                            || tokenType == TokenType.LV_VARIABLE) {
                        tokens.add(
                                new SubToken(
                                        TokenType.ASSIGNMENT_OPERATOR,
                                        op.toString()));
                        continue;
                    }
                }

                String operator = op.toString();

                if (!tokenParser.COMPARISON_OPERATORS.contains(operator)) {
                    return new Issue.Builder()
                            .addLine(line)
                            .setType(IssueType.ERROR)
                            .setIssueMessage(FDAIssueMessage.INVALID_OPERATOR_SEQUENCE_IN_LINE +
                                    line.getLineNo())
                            .build();
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
                                || lineData.charAt(i) == '_'
                                || lineData.charAt(i) == ':'
                                || (TRACE_ON.startsWith(word.toString()))
                                || (TRACE_OFF.startsWith(word.toString()))
                                || (END.startsWith(word.toString())))) {
                    word.append(lineData.charAt(i));
                    i++;
                }

                String value = word.toString();

                if (value.equalsIgnoreCase(AND)) {
                    tokens.add(
                            new SubToken(TokenType.AND_OPERATOR,
                                    value));
                } else if (value.equalsIgnoreCase(OR)) {
                    tokens.add(
                            new SubToken(TokenType.OR_OPERATOR,
                                    value));
                } else if (value.equalsIgnoreCase(EXITSCRIPT)) {
                    tokens.add(
                            new SubToken(TokenType.EXITSCRIPT,
                                    value));
                } else if (value.equalsIgnoreCase(TRACE_ON)) {
                    tokens.add(
                            new SubToken(TokenType.TRACE_ON,
                                    value));
                } else if (value.equalsIgnoreCase(TRACE_OFF)) {
                    tokens.add(
                            new SubToken(TokenType.TRACE_OFF,
                                    value));
                } else if (value.equalsIgnoreCase(END)) {
                    tokens.add(
                            new SubToken(TokenType.END,
                                    value));
                } else if (value.equalsIgnoreCase(LIBNAME)) {
                    tokens.add(
                            new SubToken(TokenType.LIBNAME,
                                    value));
                } else if (value.equalsIgnoreCase(IMPORT)) {
                    tokens.add(
                            new SubToken(TokenType.IMPORT,
                                    value));
                } else if (value.contains(":")) {
                    tokens.add(
                            new SubToken(TokenType.LABEL,
                                    value));
                } else if (value.equalsIgnoreCase(GOTO)) {
                    tokens.add(
                            new SubToken(TokenType.GOTO,
                                    value));
                } else if (value.equalsIgnoreCase(GOSUB)) {
                    tokens.add(
                            new SubToken(TokenType.GOSUB,
                                    value));
                } else if (value.toLowerCase().startsWith("func_")) {
                    tokens.add(
                            new SubToken(TokenType.FUNCTION, value));
                } else if (isBuiltInFunction(value)) {
                    tokens.add(
                            new SubToken(TokenType.FUNCTION, value));
                } else if (value.toLowerCase().startsWith("urhk_")) {
                    tokens.add(
                            new SubToken(TokenType.USERHOOK,
                                    value));
                } else if (value
                        .matches("^[A-Za-z_][A-Za-z0-9_]*\\.[A-Za-z_][A-Za-z0-9_]*\\.[A-Za-z_][A-Za-z0-9_]*$")) {
                    tokens.add(
                            new SubToken(TokenType.REP_VARIABLE,
                                    value));
                } else if (value.toLowerCase().startsWith("sv_")) {
                    tokens.add(
                            new SubToken(TokenType.SV_VARIABLE,
                                    value));
                } else if (value.toLowerCase().startsWith("fv_")) {
                    tokens.add(
                            new SubToken(TokenType.FV_VARIABLE,
                                    value));
                } else if (value.toLowerCase().startsWith("lv_")) {
                    tokens.add(
                            new SubToken(TokenType.LV_VARIABLE,
                                    value));
                } else if (value.equalsIgnoreCase(IF) || value.equalsIgnoreCase(THEN)
                        || value.equalsIgnoreCase(ELSE) || value.equalsIgnoreCase(ENDIF)) {
                    tokens.add(new SubToken(TokenType.IF_KEYWORDS,
                            value));
                } else if (value.equalsIgnoreCase(WHILE) || value.equalsIgnoreCase(DO)) {
                    tokens.add(new SubToken(TokenType.WHILE_KEYWORDS,
                            value));
                } else {
                    tokens.add(new SubToken(TokenType.UNKNOWN_IDENTIFIER,
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

    private boolean isBuiltInFunction(String function) throws Exception {

        if (knownFunctions == null) {
            this.knownFunctions = tokenParser.getFunctionMap();
        }

        String lcFunction = function.toLowerCase();

        for (String key : knownFunctions.keySet()) {
            String lcKey = FDAUtils.getFunctionWthotBrcks(key);

            if (lcFunction.equals(lcKey))
                return true;
        }
        return false;
    }
}
