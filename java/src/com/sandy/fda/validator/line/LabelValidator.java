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
import com.sandy.fda.parser.Tokenizer;
import com.sandy.fda.utils.FDAIssueMessage;
import com.sandy.fda.utils.FDAUtils;

public class LabelValidator {

    private Tokenizer tokenizer;
    private boolean labelsLoaded;
    private Map<String, Integer> labelInfo = new HashMap<>();

    public LabelValidator(Tokenizer tokenizer) {
        this.tokenizer = tokenizer;
    }

    @SuppressWarnings("unchecked")
    private boolean loadLabels(ScriptInfo scriptInfo) throws Exception {
        for (Line l : scriptInfo.getAllLines()) {
            if (l.getType() == LineType.LABEL) {
                Object obj = tokenizer.tokenize(l);

                if (obj instanceof Issue) {
                    scriptInfo.getIssues().add((Issue) obj);
                    continue;
                }

                List<SubToken> tokens = (List<SubToken>) obj;
                
                if (tokens.size() > 0) {
                    int idx = tokens.get(0).getValue().indexOf(":");
                    if (idx != -1) {
                        labelInfo.put(tokens.get(0).getValue().substring(0,idx), l.getLineNo());
                        continue;
                    }

                    labelInfo.put(tokens.get(0).getValue(), l.getLineNo());
                }
            }
        }
        return true;
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

        switch (line.getType()) {
            case LABEL:
                if (tokens.size() > 1) {
                    scriptInfo.getIssues().add(FDAUtils.buildUnexpectedTokenIssue(line));
                    return;
                }
                break;
            case GOTO:
            case GOSUB:
                if (!labelsLoaded)
                    labelsLoaded = loadLabels(scriptInfo);

                if (tokens.size() > 2) {
                    scriptInfo.getIssues().add(FDAUtils.buildUnexpectedTokenIssue(line));
                    return;
                }

                String label = tokens.get(1).getValue();
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
