package com.sandy.fda.models;

import java.util.ArrayList;
import java.util.List;

public class RClass {

    private Repository repository;
    private int classType;
    private List<Variable> variables;

    public void setRepository(Repository repository) {

        this.repository = repository;
    }

    private RClass(Builder builder) {
        this.classType = builder.classType;
        this.variables = builder.variables;
    }

    public Repository getRepository() {
        return repository;
    }

    public int getClassType() {
        return classType;
    }

    public List<Variable> getVariables() {
        return variables;
    }

    public static class Builder {

        private int classType;
        private List<Variable> variables = new ArrayList<>();

        public Builder setClassType(int classType) {
            this.classType = classType;
            return this;
        }

        public Builder addVariable(Variable variable) {
            variables.add(variable);
            return this;
        }

        public RClass build() {
            RClass cls = new RClass(this);

            for (Variable v : variables) {
                v.setRClass(cls);
            }

            return cls;
        }
    }
}