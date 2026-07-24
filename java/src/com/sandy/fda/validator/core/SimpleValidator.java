package com.sandy.fda.validator.core;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.EnumMap;
import java.util.List;
import java.util.Map;

import com.sandy.fda.models.Issue;
import com.sandy.fda.models.Line;
import com.sandy.fda.models.ScriptInfo;
import com.sandy.fda.models.enums.IssueType;
import com.sandy.fda.models.enums.LineType;
import com.sandy.fda.utils.FDAIssueMessage;

public class SimpleValidator {

    public void validate(ScriptInfo scriptInfo) {
        List<Line> allLines = scriptInfo.getAllLines();
        Map<LineType, BasicValidationInfo> basicValidationMap = new EnumMap<>(LineType.class);

        for (Line line : allLines) {
            if (line.getType() == LineType.START || line.getType() == LineType.END
                    || line.getType() == LineType.TRACEON || line.getType() == LineType.TRACEOFF
                    || line.getType() == LineType.EXITSCRIPT) {

                BasicValidationInfo info = basicValidationMap.get(line.getType());

                if (info == null) {
                    info = new BasicValidationInfo();
                    basicValidationMap.put(line.getType(), info);
                }

                info.add(line);
            }
        }

        collectBasicIssues(basicValidationMap, scriptInfo.getIssues());
    }

    private void collectBasicIssues(
            Map<LineType, BasicValidationInfo> bvMap,
            List<Issue> issues) {

        BasicValidationInfo startInfo = bvMap.get(LineType.START);

        if (startInfo == null) {
            issues.add(new Issue.Builder()
                    .setType(IssueType.ERROR)
                    .setIssueMessage(FDAIssueMessage.START_NOT_FOUND)
                    .build());

        } else if (startInfo.getCount() > 1) {
            issues.add(new Issue.Builder()
                    .setType(IssueType.ERROR)
                    .setIssueMessage(buildMsg(
                            FDAIssueMessage.MULTIPLE_START_FOUND,
                            startInfo.getLines()))
                    .setLines(startInfo.getLines())
                    .build());
        }

        BasicValidationInfo endInfo = bvMap.get(LineType.END);

        if (endInfo == null) {
            issues.add(new Issue.Builder()
                    .setType(IssueType.WARNING)
                    .setIssueMessage(FDAIssueMessage.END_NOT_FOUND)
                    .build());

        } else if (endInfo.getCount() > 1) {
            issues.add(new Issue.Builder()
                    .setType(IssueType.ERROR)
                    .setIssueMessage(buildMsg(
                            FDAIssueMessage.MULTIPLE_END_FOUND,
                            endInfo.getLines()))
                    .setLines(endInfo.getLines())
                    .build());
        }

        BasicValidationInfo traceOnInfo = bvMap.get(LineType.TRACEON);

        if (traceOnInfo == null) {
            issues.add(new Issue.Builder()
                    .setType(IssueType.WARNING)
                    .setIssueMessage(FDAIssueMessage.TRACE_ON_NOT_FOUND)
                    .build());

        } else if (traceOnInfo.getCount() > 1) {
            issues.add(new Issue.Builder()
                    .setType(IssueType.WARNING)
                    .setIssueMessage(buildMsg(
                            FDAIssueMessage.MULTIPLE_TRACE_ON_FOUND,
                            traceOnInfo.getLines()))
                    .setLines(traceOnInfo.getLines())
                    .build());
        }

        BasicValidationInfo traceOffInfo = bvMap.get(LineType.TRACEOFF);

        if (traceOffInfo == null) {
            issues.add(new Issue.Builder()
                    .setType(IssueType.WARNING)
                    .setIssueMessage(FDAIssueMessage.TRACE_OFF_NOT_FOUND)
                    .build());

        } else if (traceOffInfo.getCount() > 1) {
            issues.add(new Issue.Builder()
                    .setType(IssueType.WARNING)
                    .setIssueMessage(buildMsg(
                            FDAIssueMessage.MULTIPLE_TRACE_OFF_FOUND,
                            traceOffInfo.getLines()))
                    .setLines(traceOffInfo.getLines())
                    .build());
        }

        BasicValidationInfo exitInfo = bvMap.get(LineType.EXITSCRIPT);

        if (exitInfo == null) {
            issues.add(new Issue.Builder()
                    .setType(IssueType.WARNING)
                    .setIssueMessage(FDAIssueMessage.EXITSCRIPT_NOT_FOUND)
                    .build());

        } else if (exitInfo.getCount() > 1) {
            issues.add(new Issue.Builder()
                    .setType(IssueType.WARNING)
                    .setIssueMessage(buildMsg(
                            FDAIssueMessage.MULTIPLE_EXITSCRIPT_FOUND,
                            exitInfo.getLines()))
                    .setLines(exitInfo.getLines())
                    .build());
        }

        if (startInfo != null && endInfo != null
                && startInfo.getCount() == 1
                && endInfo.getCount() == 1) {

            Line startLine = startInfo.getLines().get(0);
            Line endLine = endInfo.getLines().get(0);

            if (startLine.getLineNo() > endLine.getLineNo()) {
                issues.add(new Issue.Builder()
                        .setType(IssueType.ERROR)
                        .setIssueMessage(FDAIssueMessage.START_FOUND_AFTER_END)
                        .setLines(Arrays.asList(startLine, endLine))
                        .build());
            }
        }

        if (traceOnInfo != null && traceOffInfo != null
                && traceOnInfo.getCount() == 1
                && traceOffInfo.getCount() == 1) {

            Line traceOnLine = traceOnInfo.getLines().get(0);
            Line traceOffLine = traceOffInfo.getLines().get(0);

            if (traceOnLine.getLineNo() > traceOffLine.getLineNo()) {
                issues.add(new Issue.Builder()
                        .setType(IssueType.ERROR)
                        .setIssueMessage(FDAIssueMessage.TRACE_ON_FOUND_AFTER_TRACE_OFF)
                        .setLines(Arrays.asList(traceOnLine, traceOffLine))
                        .build());
            }
        }
    }

    private String buildMsg(String base, List<Line> lines) {

        StringBuilder sb = new StringBuilder(base);
        sb.append(" at lines: ");

        for (Line l : lines) {
            sb.append(l.getLineNo()).append(",");
        }

        if (!lines.isEmpty()) {
            sb.setLength(sb.length() - 1);
        }

        return sb.toString();
    }

    private static class BasicValidationInfo {

        private int count;
        private List<Line> lines = new ArrayList<>();

        void add(Line line) {
            count++;
            lines.add(line);
        }

        int getCount() {
            return count;
        }

        List<Line> getLines() {
            return lines;
        }
    }

}
