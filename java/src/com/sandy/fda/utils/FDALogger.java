package com.sandy.fda.utils;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.nio.file.Files;
import java.time.LocalDateTime;

public class FDALogger {

    private static String logPath;
    private static boolean initialized = false;

    private FDALogger() {
    }

    public static boolean initLogger() {

        try {

            String userHome = System.getProperty("user.home");

            File logDir = new File(userHome, "FDA/logs");

            if (!logDir.exists()) {
                logDir.mkdirs();
            }

            logPath = new File(
                    logDir,
                    "fda-java.log").getAbsolutePath();

            Files.setAttribute(
                    logDir.toPath(),
                    "dos:hidden",
                    true);

            initialized = true;

            info("======================================");
            info("FinacleDevAssist Logger Initialized");
            info("======================================");

            return true;

        } catch (Exception e) {

            System.err.println(
                    "Logger initialization failed : "
                            + e.getMessage());

            return false;
        }
    }

    public static void info(String message) {
        log("INFO", message);
    }

    public static void error(Exception e) {
        log(
                "EXCEPTION",
                e.getClass().getName()
                        + " : "
                        + e.getMessage());

        StringWriter sw = new StringWriter();
        PrintWriter pw = new PrintWriter(sw);

        e.printStackTrace(pw);

        log("STACK TRACE", sw.toString());
    }

    private static void log(String level, String msg) {

        if (!initialized) {
            System.out.println(
                    "Logger not initialized : "
                            + msg);
            return;
        }

        String message = "[" + LocalDateTime.now() + "] "
                + "[" + level + "] "
                + msg;

        try (FileWriter writer = new FileWriter(logPath, true)) {

            writer.write(message);
            writer.write(System.lineSeparator());

        } catch (IOException e) {
            System.err.println(
                    "Unable to write log : "
                            + e.getMessage());
        }
    }
}