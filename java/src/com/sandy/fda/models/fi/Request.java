package com.sandy.fda.models.fi;

import java.util.Map;

import com.sandy.fda.models.fi.enums.HttpMethod;

public class Request {
    private String endpoint;
    private HttpMethod method;
    private Map<String, String> headers;
    private String body;

    public String getEndpoint() {
        return endpoint;
    }

    public HttpMethod getMethod() {
        return method;
    }

    public Map<String, String> getHeaders() {
        return headers;
    }

    public String getBody() {
        return body;
    }
}