package com.sandy.fda.models.fi;

public class FIException extends Exception {

    public FIException(Exception exception, String message) {
        super(message, exception);
    }

}