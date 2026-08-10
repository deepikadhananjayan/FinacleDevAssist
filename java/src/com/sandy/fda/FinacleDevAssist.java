package com.sandy.fda;

import com.sandy.fda.socket.FDASocket;
import com.sandy.fda.utils.FDAConstants;
import com.sandy.fda.utils.FDALogger;

public class FinacleDevAssist {

    private static final FinacleDevAssist INSTANCE = new FinacleDevAssist();

    private FinacleDevAssist() {
    }

    public static FinacleDevAssist getInstance() {
        return INSTANCE;
    }

    public void start() {
        if (!FDALogger.initLogger()) {
            System.err.println(
                    "Logger initialization failed. Continuing application startup.");
        }

        if (!FDAConstants.load()) {
            throw new IllegalStateException(
                    "Application startup failed: Properties could not be loaded. Please verify that the properties file exists and is accessible.");
        }

        new FDASocket().initSocket();
    }

    public static void main(String[] args) {
        getInstance().start();
    }
}