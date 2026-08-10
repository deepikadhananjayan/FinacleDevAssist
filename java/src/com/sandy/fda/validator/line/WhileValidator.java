package com.sandy.fda.validator.line;

import java.util.List;

import com.sandy.fda.models.validator.Issue;
import com.sandy.fda.models.validator.Line;
import com.sandy.fda.models.validator.ScriptInfo;
import com.sandy.fda.models.validator.SubToken;
import com.sandy.fda.models.validator.enums.IssueType;
import com.sandy.fda.models.validator.enums.TokenType;
import com.sandy.fda.parser.TokenParser;
import com.sandy.fda.utils.FDAUtils;
import com.sandy.fda.validator.line.rules.ConditionValidator;

public class WhileValidator {

    private ConditionValidator conditionValidator;

    public WhileValidator(TokenParser tokenParser, ConditionValidator conditionValidator) {
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
            case WHILE:
                if (lastToken.getType() != TokenType.CLOSE_BRACKET) {
                    scriptInfo.getIssues().add(FDAUtils.buildUnexpectedTokenIssue(line));
                    return;
                }

                tokens.remove(0);

                conditionValidator.validate(line, tokens, scriptInfo);
                break;
            case DO:
                if (!lastToken.getValue().equals("DO")) {
                    scriptInfo.getIssues().add(FDAUtils.buildUnexpectedTokenIssue(line));
                    return;
                }
                break;
            default:
                break;
        }

    }
}
