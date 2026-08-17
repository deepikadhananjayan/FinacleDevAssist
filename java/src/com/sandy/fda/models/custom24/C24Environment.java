package com.sandy.fda.models.custom24;

public record C24Environment(
        String name,
        String host,
        int port,
        String username,
        String password,
        String bankId,
        String bePath,
        String fePath,
        String xmlPath,
        String propsPath,
        String incGincPath,
        String jsPath,
        String linkGlinkPath,
        String infengPath,
        String helpPath,
        String sqlPath,
        String scriptPath
) {
}