package com.sandy.fda.models;

import com.sandy.fda.models.enums.DataType;

public class Variable {

    private RClass rClass;
    private DataType type;
    private String name;
    private String value;

    public void setRClass(RClass rClass) {
        this.rClass = rClass;
    }

    public RClass getrClass() {
        return rClass;
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

    public Variable(
            DataType type,
            String name,
            String value) {

        this.type = type;
        this.name = name;
        this.value = value;
    }
}