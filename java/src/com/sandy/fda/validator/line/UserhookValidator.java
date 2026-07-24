package com.sandy.fda.validator.line;

import java.util.List;
import java.util.Map;

import com.sandy.fda.models.Issue;
import com.sandy.fda.models.Line;
import com.sandy.fda.models.ScriptInfo;
import com.sandy.fda.models.SubToken;
import com.sandy.fda.models.Token;
import com.sandy.fda.models.enums.IssueType;
import com.sandy.fda.parser.TokenParser;
import com.sandy.fda.utils.FDAIssueMessage;
import com.sandy.fda.utils.ValidatorUtil;

public class UserhookValidator {

    private TokenParser tokenParser;

    private Map<String, Token> knownUserhooks;

    public UserhookValidator(TokenParser tokenParser) {
        this.tokenParser = tokenParser;
    }

    public void validate(Line line, List<SubToken> tokens, ScriptInfo scriptInfo) throws Exception {

        if (knownUserhooks == null) {
            this.knownUserhooks = tokenParser.getUserhookMap();
        }

        String lineContent = ValidatorUtil.rmvCmtNdTrlgSpc(line.getLineContent());

        if (!lineContent.matches(".*\\)$")) {
            scriptInfo.getIssues().add(ValidatorUtil.buildUnexpectedTokenIssue(line));
            return;
        }

        Issue issue = inspectUserhook(line);

        if (issue != null) {
            scriptInfo.getIssues().add(issue);    
        }

        return;
    }

    private Issue inspectUserhook(Line line) {
        String lcUserhook = getUserhookWthotBrcks(line.getLineContent());
        Token userhook = null;

        for (String key : knownUserhooks.keySet()) {
            String lcKey = getUserhookWthotBrcks(key);

            if (lcUserhook.equals(lcKey)) {
                userhook = knownUserhooks.get(key);
                break;
            }
        }

        if (userhook == null) {
            return new Issue.Builder()
                    .addLine(line)
                    .setType(IssueType.WARNING)
                    .setIssueMessage(FDAIssueMessage.USERHOOK_NOT_REGISTERED + line.getLineNo())
                    .build();

        }
        // handle parameter checks and all
        return null;
    }

    private String getUserhookWthotBrcks(String urhk) {
        int bgnIdx = urhk.toLowerCase().indexOf("urhk");
        int endIdx = urhk.toLowerCase().indexOf("(", bgnIdx);

        return urhk.substring(bgnIdx, endIdx).toLowerCase();
    }
}
