package com.sandy.fda.validator.line;

import java.util.List;

import com.sandy.fda.models.validator.Issue;
import com.sandy.fda.models.validator.Line;
import com.sandy.fda.models.validator.ScriptInfo;
import com.sandy.fda.models.validator.SubToken;
import com.sandy.fda.models.validator.enums.IssueType;
import com.sandy.fda.parser.TokenParser;
import com.sandy.fda.utils.FDAIssueMessage;
import com.sandy.fda.utils.FDAUtils;
import com.sandy.fda.validator.line.rules.ConditionValidator;

public class IfValidator {

    private ConditionValidator conditionValidator;

    public IfValidator(TokenParser tokenParser, ConditionValidator conditionValidator) {
        this.conditionValidator = conditionValidator;
    }

    public void validate(Line line, List<SubToken> tokens, ScriptInfo scriptInfo) throws Exception {

        int lastIdx = tokens.size() - 1; 
        if (lastIdx == -1) {
            scriptInfo.getIssues().add(new Issue.Builder().addLine(line)
                            .setIssueMessage("Tokenize issue in line " + line.getLineNo())
                            .setType(IssueType.ERROR)
                            .build());
            return;
        }

        SubToken lastToken = tokens.get(lastIdx);

        switch (line.getType()) {
            case IF:{
                if (!lastToken.getValue().equals("THEN")) {
                    scriptInfo.getIssues().add(new Issue.Builder().addLine(line)
                            .setIssueMessage(FDAIssueMessage.THEN_MISSING_IN_IF)
                            .setType(IssueType.ERROR)
                            .build());
                    return;
                }

                tokens.remove(lastIdx);
                tokens.remove(0);

                conditionValidator.validate(line, tokens, scriptInfo);
                break;
            }
            case ELSE:
                if (!lastToken.getValue().equals("ELSE")) {
                    scriptInfo.getIssues().add(FDAUtils.buildUnexpectedTokenIssue(line));
                    return;
                }
                break;
            case ENDIF:
                if (!lastToken.getValue().equals("ENDIF")) {
                    scriptInfo.getIssues().add(FDAUtils.buildUnexpectedTokenIssue(line));
                    return;
                }
                break;
            default:
                break;
        }
    }
}
