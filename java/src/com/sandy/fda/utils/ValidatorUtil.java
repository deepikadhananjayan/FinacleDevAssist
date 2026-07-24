package com.sandy.fda.utils;

import com.sandy.fda.models.Issue;
import com.sandy.fda.models.Line;
import com.sandy.fda.models.enums.IssueType;

public class ValidatorUtil {

    public static Issue buildUnexpectedTokenIssue(Line line) {
        return new Issue.Builder().addLine(line)
                .setIssueMessage(FDAIssueMessage.UNEXPECTED_TOKEN_IN_LINE + line.getLineNo())
                .setType(IssueType.ERROR)
                .build();
    }

    public static String rmvCmtNdTrlgSpc(String lineContent) {
        int cmtPos = lineContent.indexOf("#");
        
        if (cmtPos == -1) {
            return lineContent.trim();
        }

        return lineContent.substring(0, cmtPos).trim();
    }
}
