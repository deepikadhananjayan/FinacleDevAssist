package com.sandy.fda.validator.line;

import java.util.List;

import com.sandy.fda.models.Issue;
import com.sandy.fda.models.Line;
import com.sandy.fda.models.ScriptInfo;
import com.sandy.fda.models.SubToken;
import com.sandy.fda.models.enums.IssueType;
import com.sandy.fda.parser.TokenParser;
import com.sandy.fda.utils.FDAIssueMessage;
import com.sandy.fda.utils.ValidatorUtil;
import com.sandy.fda.validator.line.rules.ConditionValidator;

public class IfValidator {

    private ConditionValidator conditionValidator;

    public IfValidator(TokenParser tokenParser, ConditionValidator conditionValidator){
        this.conditionValidator = conditionValidator;
    }

    public void validate(Line line, List<SubToken> tokens,ScriptInfo scriptInfo) throws Exception{
        String lineContent = ValidatorUtil.rmvCmtNdTrlgSpc(line.getLineContent());

        switch (line.getType()) {
            case IF:
                if (!lineContent.matches(".*\\bTHEN\\b$")) {
                    scriptInfo.getIssues().add(new Issue.Builder().addLine(line)
                            .setIssueMessage(FDAIssueMessage.THEN_MISSING_IN_IF)
                            .setType(IssueType.ERROR)
                            .build());
                    return;
                }
                conditionValidator.validate(line, tokens, scriptInfo);
                break;
            case ELSE:
                if (!lineContent.matches("^ELSE\\b$")) {
                    scriptInfo.getIssues().add(ValidatorUtil.buildUnexpectedTokenIssue(line));
                    return;
                }
                break;
            case ENDIF:
                if (!lineContent.matches("^ENDIF\\b$")) {
                    scriptInfo.getIssues().add(ValidatorUtil.buildUnexpectedTokenIssue(line));
                    return;
                }
                break;
            default:
                break;
        }
    }
}
