package com.sandy.fda.models;

import com.sandy.fda.models.enums.TokenType;

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
    }