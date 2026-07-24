package com.sandy.fda.models;

import java.util.ArrayList;
import java.util.List;

public class Repository {

    private String name;
    private List<RClass> classes;

    private Repository(Builder builder) {
        this.name = builder.name;
        this.classes = builder.classes;
    }

    public String getName() {
        return name;
    }

    public List<RClass> getClasses() {
        return classes;
    }

    public static class Builder {

        private String name;
        private List<RClass> classes = new ArrayList<>();

        public Builder setName(String name) {
            this.name = name;
            return this;
        }

        public Builder addClass(RClass rClass) {
            classes.add(rClass);
            return this;
        }

        public Repository build() {
            Repository repo = new Repository(this);

            for (RClass cls : classes) {
                cls.setRepository(repo);
            }

            return repo;
        }
    }
}