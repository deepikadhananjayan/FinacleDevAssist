package com.sandy.fda.parser;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.sandy.fda.models.validator.Line;
import com.sandy.fda.models.validator.Token;
import com.sandy.fda.models.validator.Line.Builder;
import com.sandy.fda.models.validator.enums.LineType;
import com.sandy.fda.utils.FDAUtils;

public class ScriptParser {

    private TokenParser tokenParser;
    private Map<String, Token> knownFunctions = null;

    private static final Pattern ASSIGNMENT_PATTERN = Pattern.compile(
            "^\\s*[A-Za-z_][A-Za-z0-9_]*(?:\\.[A-Za-z_][A-Za-z0-9_]*){0,2}\\s*=\\s*.+$");

    private static final Pattern ASSIGNMENT_OPERATOR = Pattern.compile("(?<![<>=!])=(?![=])");

    public ScriptParser(TokenParser tokenParser) {
        this.tokenParser = tokenParser;
    }

    public List<Line> parse(String content, boolean isFilePath) throws Exception {

        List<String> allLines = null;

        if (isFilePath) {
            allLines = FDAUtils.getAllLines(content);
        } else {
            allLines = new ArrayList<>(
                    Arrays.asList(
                            content.split("\\r\\n|\\r|\\n", -1)));
        }

        List<Line> scrLines = new ArrayList<>();

        for (int i = 0; i < allLines.size(); i++) {
            int lineNo = (i + 1);
            String lineContent = allLines.get(i);

            Builder builder = new Line.Builder();
            builder.setLineContent(lineContent);
            builder.setLineNo(lineNo);

            lineContent = lineContent.toUpperCase().trim();

            if (!lineContent.startsWith("#") && lineContent.contains("#"))
                lineContent = lineContent.substring(0, lineContent.indexOf("#")).trim();

            if (lineContent.isEmpty())
                builder.setLineType(LineType.EMPTYLINE);
            else if ((lineContent.matches("#\\s*\\{.*")) || (lineContent.matches("#\\s*\\}.*")))
                builder.setLineType(LineType.BLOCK);
            else if (lineContent.startsWith("#"))
                builder.setLineType(LineType.COMMENTLINE);
            else if (lineContent.matches("^LIBNAME\\b\\s*.*$"))
                builder.setLineType(LineType.LIBNAME);
            else if (lineContent.matches("^IMPORT\\b\\s*.*$"))
                builder.setLineType(LineType.IMPORT);
            else if (lineContent.matches("^<--START\\s*.*"))
                builder.setLineType(LineType.START);
            else if (lineContent.matches("^END-->\\s*.*"))
                builder.setLineType(LineType.END);
            else if (lineContent.matches("^TRACE\\s+ON\\b.*"))
                builder.setLineType(LineType.TRACEON);
            else if (lineContent.matches("^TRACE\\s+OFF\\b.*"))
                builder.setLineType(LineType.TRACEOFF);
            else if (lineContent.matches("^EXITSCRIPT\\b.*"))
                builder.setLineType(LineType.EXITSCRIPT);
            else if (lineContent.matches("^\\s*[A-Za-z_][A-Za-z0-9_]*\\s*:[^:]*$"))
                builder.setLineType(LineType.LABEL);
            else if (lineContent.matches("^IF\\s*\\(.*\\)\\s*(THEN\\b.*)?$"))
                builder.setLineType(LineType.IF);
            else if (lineContent.matches("^ELSE\\b.*$"))
                builder.setLineType(LineType.ELSE);
            else if (lineContent.matches("^ENDIF\\b.*$"))
                builder.setLineType(LineType.ENDIF);
            else if (lineContent.matches("^WHILE\\s*\\(.*\\).*$"))
                builder.setLineType(LineType.WHILE);
            else if (lineContent.matches("^DO\\b.*$"))
                builder.setLineType(LineType.DO);
            else if (lineContent.matches(".*URHK_.*\\(.*\\).*"))
                builder.setLineType(LineType.USERHOOK);
            else if (lineContent.matches(".*FUNC_.*\\(.*\\).*") || isBuiltInFunction(lineContent))
                builder.setLineType(LineType.FUNCTION_CALL);
            else if (isAssignment(lineContent))
                builder.setLineType(LineType.ASSIGNMENT);
            else if (lineContent.matches("^GOTO\\b.*"))
                builder.setLineType(LineType.GOTO);
            else if (lineContent.matches("^GOTO\\b.*"))
                builder.setLineType(LineType.GOSUB);
            else
                builder.setLineType(LineType.UNKNOWN);

            scrLines.add(builder.build());
        }
        return scrLines;
    }

    private boolean isAssignment(String lineContent) {
        Matcher structure = ASSIGNMENT_PATTERN.matcher(lineContent);

        if (!structure.matches()) {
            return false;
        }

        Matcher operator = ASSIGNMENT_OPERATOR.matcher(lineContent);

        int count = 0;
        while (operator.find()) {
            count++;
        }

        return count == 1;
    }

    private boolean isBuiltInFunction(String lineContent) throws Exception {

        if (knownFunctions == null) {
            this.knownFunctions = tokenParser.getFunctionMap();
        }

        String lcContent = lineContent.toLowerCase();

        for (String key : knownFunctions.keySet()) {
            String lcKey = FDAUtils.getFunctionWthotBrcks(key);

            if (lcContent.matches(".*" + Pattern.quote(lcKey) + "\\(.*\\).*"))
                return true;
        }
        return false;
    }

}
