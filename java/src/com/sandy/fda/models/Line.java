package com.sandy.fda.models;

import com.sandy.fda.models.enums.LineType;

public class Line {
    private LineType type;
    private int lineNo;
    private String lineContent;

    private Line() {
    }

    private Line(Builder builder) {
        this.type = builder.type;
        this.lineNo = builder.lineNo;
        this.lineContent = builder.lineContent;
    }

    public LineType getType() {
        return type;
    }

    public int getLineNo() {
        return lineNo;
    }

    public String getLineContent() {
        return lineContent;
    }

    public static class Builder {
        private LineType type;
        private int lineNo;
        private String lineContent;

        public Builder setLineType(LineType type) {
            this.type = type;
            return this;
        }

        public Builder setLineNo(int lineNo) {
            this.lineNo = lineNo;
            return this;
        }

        public Builder setLineContent(String lineContent) {
            this.lineContent = lineContent;
            return this;
        }

        public Line build() {
            if (this.type == null) {
                throw new IllegalStateException("Line Type missing");
            }
            return new Line(this);
        }
    }
}
