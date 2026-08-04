package com.sandy.fda;

import com.sandy.fda.socket.FDASocket;
import com.sandy.fda.utils.FDAConstants;
import com.sandy.fda.utils.FDALogger;

public class FinacleDevAssist {

    private static final FinacleDevAssist INSTANCE = new FinacleDevAssist();

    private FDASocket fdaSocket;

    private FinacleDevAssist() {
        fdaSocket = new FDASocket();
    }

    public static FinacleDevAssist getInstance() {
        return INSTANCE;
    }

    public void start() {
        FDALogger.initLogger();

        FDAConstants.load();

        fdaSocket.initSocket();
    }

    public static void main(String[] args) {
        getInstance().start();
    }
}