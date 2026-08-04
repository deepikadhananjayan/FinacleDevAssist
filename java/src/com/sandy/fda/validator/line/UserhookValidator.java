package com.sandy.fda.validator.line;

import java.util.List;
import java.util.Map;

import com.sandy.fda.models.Issue;
import com.sandy.fda.models.Line;
import com.sandy.fda.models.ScriptInfo;
import com.sandy.fda.models.SubToken;
import com.sandy.fda.models.Token;
import com.sandy.fda.models.enums.IssueType;
import com.sandy.fda.models.enums.TokenType;
import com.sandy.fda.parser.TokenParser;
import com.sandy.fda.utils.FDAIssueMessage;
import com.sandy.fda.utils.FDAUtils;

public class UserhookValidator {

    private TokenParser tokenParser;

    private Map<String, Token> knownUserhooks;

    public UserhookValidator(TokenParser tokenParser) {
        this.tokenParser = tokenParser;
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

        if (lastToken.getType() != TokenType.CLOSE_BRACKET) {
            scriptInfo.getIssues().add(FDAUtils.buildUnexpectedTokenIssue(line));
            return;
        }

        if (knownUserhooks == null) {
            this.knownUserhooks = tokenParser.getUserhookMap();
        }

        for (int i = 0; i < tokens.size(); i++) {
            if (tokens.get(i).getType() == TokenType.USERHOOK) {
                inspectUserhook(tokens.get(i).getValue(), tokens, line, scriptInfo);
            }
        }
    }

    private void inspectUserhook(String userhook, List<SubToken> tokens, Line line, ScriptInfo scriptInfo) {
        String lcUserhook = userhook.toLowerCase();
        Token urhk = null;

        for (String key : knownUserhooks.keySet()) {
            String lcKey = FDAUtils.getFunctionWthotBrcks(key);

            if (lcUserhook.equals(lcKey)) {
                urhk = knownUserhooks.get(key);
                break;
            }
        }

        if (urhk == null) {
            scriptInfo.getIssues().add(new Issue.Builder()
                    .addLine(line)
                    .setType(IssueType.WARNING)
                    .setIssueMessage(FDAIssueMessage.USERHOOK_NOT_REGISTERED + line.getLineNo())
                    .build());

        }
    }
}
