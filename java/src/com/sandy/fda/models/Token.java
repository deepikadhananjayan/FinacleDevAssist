package com.sandy.fda.models;

import java.util.List;

import com.sandy.fda.models.enums.TokenType;

public class Token {
    private String token;
    private TokenType type;

    private List<String> paramInputs;
    private List<String> inputs;
    private List<String> outputs;

    private Token() {
    }

    private Token(Builder builder) {
        this.token = builder.token;
        this.type = builder.type;
        this.paramInputs = builder.paramInputs;
        this.inputs = builder.inputs;
        this.outputs = builder.outputs;
    }

    public String getToken() {
        return token;
    }

    public TokenType getType() {
        return type;
    }

    public List<String> getParamInputs() {
        return paramInputs;
    }

    public List<String> getInputs() {
        return inputs;
    }

    public List<String> getOutputs() {
        return outputs;
    }

    public static class Builder {
        private String token;
        private TokenType type;

        private List<String> paramInputs;
        private List<String> inputs;
        private List<String> outputs;

        public Builder setToken(String token) {
            this.token = token;
            return this;
        }

        public Builder setType(TokenType type) {
            this.type = type;
            return this;
        }

        public Builder setParamInputs(List<String> paramInputs) {
            this.paramInputs = paramInputs;
            return this;
        }

        public Builder setInputs(List<String> inputs) {
            this.inputs = inputs;
            return this;
        }

        public Builder setOutputs(List<String> outputs) {
            this.outputs = outputs;
            return this;
        }

        public Token build(){
            if (this.type == null) {
                throw new IllegalStateException("Token Type missing");
            }
            
            return new Token(this);
        }

    }
}
