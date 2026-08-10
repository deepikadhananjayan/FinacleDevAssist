package com.sandy.fda.models.validator;

import com.sandy.fda.models.validator.enums.DataType;

public class ScrachPadVariable {

    private DataType type;
    private String name;
    private String value;

    private ScrachPadVariable() {
    }

    private ScrachPadVariable(Builder builder) {
        this.type = builder.type;
        this.name = builder.name;
        this.value = builder.value;
    }

    public DataType getType() {
        return type;
    }

    public String getName() {
        return name;
    }

    public String getValue() {
        return value;
    }

    public static class Builder {

        private DataType type;
        private String name;
        private String value;

        public Builder setType(DataType type) {
            this.type = type;
            return this;
        }

        public Builder setName(String name) {
            this.name = name;
            return this;
        }

        public Builder setValue(String value) {
            this.value = value;
            return this;
        }

        public ScrachPadVariable build() {

            if (this.type == null) {
                throw new IllegalStateException(
                        "Variable Type missing");
            }

            if (this.name == null || this.name.isEmpty()) {
                throw new IllegalStateException(
                        "Variable Name missing");
            }

            return new ScrachPadVariable(this);
        }
    }
}