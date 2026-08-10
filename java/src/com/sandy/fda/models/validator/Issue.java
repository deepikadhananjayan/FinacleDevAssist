package com.sandy.fda.models.validator;

import java.util.ArrayList;
import java.util.List;

import com.sandy.fda.models.validator.enums.IssueType;

@SuppressWarnings("null")
public class Issue implements Comparable<Issue> {

    private List<Line> lines;
    private IssueType type;
    private String issueMessage;

    private Issue() {
    }

    private Issue(Builder builder) {
        this.lines = builder.lines;
        this.type = builder.type;
        this.issueMessage = builder.issueMessage;
    }

    public List<Line> getLines() {
        return lines;
    }

    public IssueType getType() {
        return type;
    }

    public String getIssueMessage() {
        return issueMessage;
    }

    @Override
    public int compareTo(Issue otherIssue) {

        boolean thisSingle = lines != null && lines.size() == 1;
        boolean otherSingle = otherIssue != null && otherIssue.getLines().size() == 1;

        if (thisSingle && otherSingle) {
            return Integer.compare(
                    lines.get(0).getLineNo(),
                    otherIssue.getLines().get(0).getLineNo());
        }

        if (thisSingle) {
            return -1;
        }

        if (otherSingle) {
            return 1;
        }

        return 0;
    }

    public static class Builder {

        private List<Line> lines;
        private IssueType type;
        private String issueMessage;

        public Builder setLines(List<Line> lines) {
            this.lines = lines;
            return this;
        }

        public Builder addLine(Line line) {
            if (lines == null) {
                lines = new ArrayList<>();
            }
            lines.add(line);
            return this;
        }

        public Builder setType(IssueType type) {
            this.type = type;
            return this;
        }

        public Builder setIssueMessage(String issueMessage) {
            this.issueMessage = issueMessage;
            return this;
        }

        public Issue build() {

            if (this.type == null) {
                throw new IllegalStateException("Issue Type missing");
            }

            return new Issue(this);
        }
    }
}