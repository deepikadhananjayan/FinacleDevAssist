package com.sandy.fda.models.validator;

import com.sandy.fda.models.validator.enums.TokenType;

public class SubToken {
    private TokenType type;
    private String value;

    public SubToken(TokenType type, String value) {
        this.type = type;
        this.value = value;
    }

    public TokenType getType() {
        return type;
    }

    public String getValue() {
        return value;
    }

    @Override
    public String toString() {
        return this.getType() + " ->" + this.getValue();
    }
}