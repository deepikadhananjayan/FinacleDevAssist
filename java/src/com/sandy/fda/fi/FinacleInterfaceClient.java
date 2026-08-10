package com.sandy.fda.fi;

import com.sandy.fda.models.fi.FIException;
import com.sandy.fda.models.fi.Request;
import com.sandy.fda.models.fi.Response;
import com.sandy.fda.utils.FDALogger;

import java.io.IOException;
import java.net.ConnectException;
import java.net.URI;
import java.net.UnknownHostException;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpTimeoutException;
import java.security.SecureRandom;
import java.time.Duration;
import java.util.Map;

import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;
import java.security.cert.X509Certificate;

public class FinacleInterfaceClient {

    private final HttpClient httpClient;

    private final String TIMEOUT_EXCP_MSG = "The Finacle server did not respond within the allowed time.";
    private final String CONNECT_EXCP_MSG = "Unable to connect to the Finacle server. Please check the network connection and server availability.";
    private final String UNKNOWN_HOST_EXCP_MSG = "Unable to access the Finacle server. The server address could not be resolved.";
    private final String IO_EXCP_MSG = "A network error occurred while accessing the Finacle server.";
    private final String INTERRUPT_EXCP_MSG = "The Finacle request was interrupted.";

    public FinacleInterfaceClient() throws Exception {

        SSLContext sslContext = disableSSLVerification();

        this.httpClient = HttpClient.newBuilder()
                .sslContext(sslContext)
                .connectTimeout(Duration.ofSeconds(10))
                .build();
    }

    public Response send(Request request) throws FIException {
    try {
        HttpRequest.Builder builder = HttpRequest.newBuilder()
                .uri(URI.create(request.getEndpoint()))
                .timeout(Duration.ofSeconds(30));

        if (request.getHeaders() != null) {
            for (Map.Entry<String, String> header : request.getHeaders().entrySet()) {
                builder.header(header.getKey(), header.getValue());
            }
        }

        switch (request.getMethod()) {
            case GET -> builder.GET();
            case DELETE -> builder.DELETE();
            case POST -> builder.POST(HttpRequest.BodyPublishers.ofString(request.getBody()));
            case PUT -> builder.PUT(HttpRequest.BodyPublishers.ofString(request.getBody()));
            default -> {
                FDALogger.info("Unknown Request Method: " + request.getMethod());
            }
        }

        HttpResponse<String> httpResponse = httpClient.send(
                builder.build(),
                HttpResponse.BodyHandlers.ofString()
        );

        return new Response(httpResponse.statusCode(), httpResponse.body());

    } catch (HttpTimeoutException e) {
        FDALogger.error(e);
        throw new FIException(e, TIMEOUT_EXCP_MSG);

    } catch (ConnectException e) {
        FDALogger.error(e);
        throw new FIException(e, CONNECT_EXCP_MSG);

    } catch (UnknownHostException e) {
        FDALogger.error(e);
        throw new FIException(e, UNKNOWN_HOST_EXCP_MSG);

    } catch (IOException e) {
        FDALogger.error(e);
        throw new FIException(e, IO_EXCP_MSG);

    } catch (InterruptedException e) {
        Thread.currentThread().interrupt();
        FDALogger.error(e);
        throw new FIException(e, INTERRUPT_EXCP_MSG);
    }
}   

    private SSLContext disableSSLVerification() throws Exception {

        TrustManager[] trustAllCerts = new TrustManager[] {
                new X509TrustManager() {

                    @Override
                    public X509Certificate[] getAcceptedIssuers() {
                        return new X509Certificate[0];
                    }

                    @Override
                    public void checkClientTrusted(
                            X509Certificate[] certs,
                            String authType) {
                    }

                    @Override
                    public void checkServerTrusted(
                            X509Certificate[] certs,
                            String authType) {
                    }
                }
        };

        SSLContext sslContext = SSLContext.getInstance("TLS");
        sslContext.init(null, trustAllCerts, new SecureRandom());

        return sslContext;
    }
}