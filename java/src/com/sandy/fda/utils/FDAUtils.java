package com.sandy.fda.utils;

import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.List;

import com.sandy.fda.models.Issue;
import com.sandy.fda.models.Line;
import com.sandy.fda.models.enums.IssueType;

public class FDAUtils {

    public static Issue buildUnexpectedTokenIssue(Line line) {
        return new Issue.Builder().addLine(line)
                .setIssueMessage(FDAIssueMessage.UNEXPECTED_TOKEN_IN_LINE + line.getLineNo())
                .setType(IssueType.ERROR)
                .build();
    }

    public static List<String> getAllLines(String filePath) throws Exception {
        return Files.readAllLines(Paths.get(filePath));
    }

    public static String getAllLinesAsString(String filePath) throws Exception {
        List<String> allLines = getAllLines(filePath);
        StringBuilder lines = new StringBuilder();
        for (String line : allLines) {
            lines.append(line);
        }
        return lines.toString();
    }

    public static String getUserhookWthotBrcks(String urhk) {
        int bgnIdx = urhk.toLowerCase().indexOf("urhk");
        int endIdx = urhk.toLowerCase().indexOf("(", bgnIdx);

        return urhk.substring(bgnIdx, endIdx).toLowerCase();
    }

    public static String getFunctionWthotBrcks(String func) {
        int endIdx = func.toLowerCase().indexOf("(");
        return func.substring(0, endIdx).toLowerCase();
    }
}
