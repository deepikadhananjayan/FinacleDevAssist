package com.sandy.fda.models.fi;

public class Response {

    private int statusCode;
    private String body;
    
    public Response( int statusCode, String body) {
        this.statusCode = statusCode;
        this.body = body;
    }

    public int getStatusCode() {
        return statusCode;
    }

    public String getBody() {
        return body;
    }
}