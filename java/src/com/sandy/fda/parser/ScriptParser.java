package com.sandy.fda.parser;

import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;

import com.sandy.fda.models.Line;
import com.sandy.fda.models.Line.Builder;
import com.sandy.fda.models.enums.LineType;

public class ScriptParser {
    public List<Line> parse(String filePath) throws Exception {

        List<String> allLines = getAllLines(filePath);

        List<Line> scrLines = new ArrayList<>();

        for (int i = 0; i < allLines.size(); i++) {
            int lineNo = (i + 1);
            String lineContent = allLines.get(i);

            Builder builder = new Line.Builder();
            builder.setLineContent(lineContent);
            builder.setLineNo(lineNo);

            if (!lineContent.startsWith("#") && lineContent.contains("#"))
                lineContent = lineContent.substring(0, lineContent.indexOf("#"));

            lineContent = lineContent.trim().toUpperCase();

            if (lineContent.isEmpty())
                builder.setLineType(LineType.EMPTYLINE);
            else if (lineContent.startsWith("#"))
                builder.setLineType(LineType.COMMENTLINE);
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
            else if (lineContent.matches("^.+:.*$"))
                builder.setLineType(LineType.LABEL);
            else if (lineContent.matches("^IF\\s*\\(.*\\)\\s*(THEN\\b.*)?$"))
                builder.setLineType(LineType.IF);
            else if (lineContent.matches("^ELSE\\b.*$"))
                builder.setLineType(LineType.ELSE);
            else if (lineContent.matches("^ENDIF\\b.*$"))
                builder.setLineType(LineType.ENDIF);
            else if (lineContent.matches("^WHILE\\s*\\(.*\\)\\.*$"))
                builder.setLineType(LineType.WHILE);
            else if (lineContent.matches("^DO\\b\\.*$"))
                builder.setLineType(LineType.DO);
            else if (lineContent.matches(".*URHK_.*\\(.*\\).*"))
                builder.setLineType(LineType.USERHOOK);
            else if (lineContent.matches(".*FUNC_.*\\(.*\\).*"))
                builder.setLineType(LineType.FUNCTION_CALL);
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

    private List<String> getAllLines(String filePath) throws Exception {
        return Files.readAllLines(Paths.get(filePath));
    }

}
