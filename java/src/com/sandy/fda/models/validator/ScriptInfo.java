package com.sandy.fda.models.validator;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class ScriptInfo {

    private List<Line> allLines;
    private List<Token> userHookUsed;
    private Set<ScrachPadVariable> spVariablesUsed;
    private Set<Repository> repositories;
    private List<Issue> issues;

    private ScriptInfo() {
    }

    private ScriptInfo(Builder builder) {
        this.allLines = builder.allLines;
        this.userHookUsed = builder.userHookUsed;
        this.spVariablesUsed = builder.spVariablesUsed;
        this.repositories = builder.repositories;
        this.issues = builder.issues;
    }

    public List<Line> getAllLines() {
        return allLines;
    }

    public List<Token> getUserHookUsed() {
        return userHookUsed;
    }

    public Set<ScrachPadVariable> getSpVariablesUsed() {
        return spVariablesUsed;
    }

    public Set<Repository> getRepositories() {
        return repositories;
    }

    public List<Issue> getIssues() {
        return issues;
    }

    public static class Builder {

        private List<Line> allLines = new ArrayList<>();
        private List<Token> userHookUsed = new ArrayList<>();
        private Set<ScrachPadVariable> spVariablesUsed = new HashSet<>();
        private Set<Repository> repositories = new HashSet<>();
        private List<Issue> issues = new ArrayList<>();

        public Builder setAllLines(List<Line> allLines) {
            this.allLines = allLines;
            return this;
        }

        public Builder setUserHookUsed(List<Token> userHookUsed) {
            this.userHookUsed = userHookUsed;
            return this;
        }

        public Builder setSpVariablesUsed(
                Set<ScrachPadVariable> spVariablesUsed) {
            this.spVariablesUsed = spVariablesUsed;
            return this;
        }

        public Builder setRepositories(
                Set<Repository> repositories) {
            this.repositories = repositories;
            return this;
        }

        public Builder setIssues(List<Issue> issues) {
            this.issues = issues;
            return this;
        }

        public Builder addLine(Line line) {
            this.allLines.add(line);
            return this;
        }

        public Builder addUserHook(Token token) {
            this.userHookUsed.add(token);
            return this;
        }

        public Builder addVariable(
                ScrachPadVariable variable) {
            this.spVariablesUsed.add(variable);
            return this;
        }

        public Builder addRepository(
                Repository repository) {
            this.repositories.add(repository);
            return this;
        }

        public Builder addIssue(Issue issue) {
            this.issues.add(issue);
            return this;
        }

        public ScriptInfo build() {
            return new ScriptInfo(this);
        }
    }
}