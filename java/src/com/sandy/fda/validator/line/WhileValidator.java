package com.sandy.fda.validator.line;

import java.util.List;

import com.sandy.fda.models.Line;
import com.sandy.fda.models.ScriptInfo;
import com.sandy.fda.models.SubToken;
import com.sandy.fda.parser.TokenParser;
import com.sandy.fda.utils.ValidatorUtil;
import com.sandy.fda.validator.line.rules.ConditionValidator;

public class WhileValidator {

    private ConditionValidator conditionValidator;

    public WhileValidator(TokenParser tokenParser, ConditionValidator conditionValidator) {
        this.conditionValidator = conditionValidator;
    }

    public void validate(Line line, List<SubToken> tokens,ScriptInfo scriptInfo) throws Exception{
        String lineContent = ValidatorUtil.rmvCmtNdTrlgSpc(line.getLineContent());

        switch (line.getType()) {
            case WHILE:
                if (!lineContent.matches(".*\\)$")) {
                    scriptInfo.getIssues().add(ValidatorUtil.buildUnexpectedTokenIssue(line));
                    return;
                }
                conditionValidator.validate(line, tokens, scriptInfo);
                break;
            case DO:
                if (!lineContent.matches("^DO\\b$")) {
                    scriptInfo.getIssues().add(ValidatorUtil.buildUnexpectedTokenIssue(line));
                    return;
                }
                break;
            default:
                break;
        }

    }
}
