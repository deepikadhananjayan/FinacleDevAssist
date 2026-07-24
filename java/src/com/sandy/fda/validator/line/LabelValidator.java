package com.sandy.fda.validator.line;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.sandy.fda.models.Issue;
import com.sandy.fda.models.Line;
import com.sandy.fda.models.ScriptInfo;
import com.sandy.fda.models.SubToken;
import com.sandy.fda.models.enums.IssueType;
import com.sandy.fda.models.enums.LineType;
import com.sandy.fda.utils.FDAIssueMessage;
import com.sandy.fda.utils.ValidatorUtil;

public class LabelValidator{

    private boolean labelsLoaded;
    private Map<String, Integer> labelInfo = new HashMap<>();

    private boolean loadLabels(ScriptInfo scriptInfo) {
        for (Line l : scriptInfo.getAllLines()) {
            if (l.getType() == LineType.LABEL) {
                String labelName = ValidatorUtil.rmvCmtNdTrlgSpc(l.getLineContent());
                labelName = labelName.substring(0, labelName.length() - 1).trim();
                labelInfo.put(labelName, l.getLineNo());
            }
        }
        return true;
    }

    public void validate(Line line, List<SubToken> tokens, ScriptInfo scriptInfo) {

        String lineContent = ValidatorUtil.rmvCmtNdTrlgSpc(line.getLineContent());
        switch (line.getType()) {
            case LABEL:
                if (!lineContent.matches(".*:$")) {
                    scriptInfo.getIssues().add(ValidatorUtil.buildUnexpectedTokenIssue(line));
                    return;
                }
                break;
            case GOTO:
            case GOSUB:
                if (!labelsLoaded)
                    labelsLoaded = loadLabels(scriptInfo);

                // Need to Use tokens
                String label = lineContent.split(" ")[1].trim();
                int labelPos = labelInfo.getOrDefault(label, -1);

                if (labelPos == -1) {
                    scriptInfo.getIssues().add(new Issue.Builder()
                            .addLine(line)
                            .setType(IssueType.ERROR)
                            .setIssueMessage(FDAIssueMessage.JUMP_TO_UNDECLARED_LABEL + line.getLineNo())
                            .build());
                    return;
                }

                if (line.getType() == LineType.GOTO && labelPos < line.getLineNo()) {
                    scriptInfo.getIssues().add(new Issue.Builder()
                            .addLine(line)
                            .setType(IssueType.ERROR)
                            .setIssueMessage(FDAIssueMessage.GOTO_LABEL_DECLARED_ABOVE + line.getLineNo())
                            .build());
                    return;
                }
                break;

            default:
                break;
        }

    }
}
