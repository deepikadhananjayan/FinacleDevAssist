package com.sandy.fda.custom24;

import java.io.File;
import java.util.List;
import java.util.Map;

import com.jcraft.jsch.ChannelSftp;
import com.jcraft.jsch.JSch;
import com.jcraft.jsch.Session;
import com.jcraft.jsch.SftpException;
import com.sandy.fda.models.custom24.C24Environment;

public class SFTPHandler {
    private JSch jsch;

    public SFTPHandler() {
        this.jsch = new JSch();
    }

    void transferFiles(C24Environment env, Map<String, List<File>> files) throws Exception {
        String username = env.username();
        String host = env.host();
        int port = env.port();
        String password = env.password();

        Session session = jsch.getSession(username, host, port);
        session.setPassword(password.getBytes());
        session.setConfig(
                "StrictHostKeyChecking",
                "no");
        session.connect();

        ChannelSftp sftp = (ChannelSftp) session.openChannel("sftp");
        sftp.connect();

        for (String remoteDirectory : files.keySet()) {
            uploadFiles(sftp, files.get(remoteDirectory), remoteDirectory);
        }
    }

    private void uploadFiles(ChannelSftp sftp, List<File> localFiles, String remoteDirectory) throws Exception {
        createDirectory(sftp, remoteDirectory);

        for (File file : localFiles) {
            sftp.put(file.getAbsolutePath(), remoteDirectory, ChannelSftp.OVERWRITE);
        }
    }

    private void createDirectory(ChannelSftp sftp, String remoteDirectory) throws Exception {

        String[] directories = remoteDirectory.split("/");
        StringBuilder currentPath = new StringBuilder();

        for (String subDir : directories) {
            if (subDir.isBlank())
                continue;

            currentPath.append("/").append(subDir);

            try {
                sftp.stat(currentPath.toString());
            } catch (SftpException e) {
                if (e.id == ChannelSftp.SSH_FX_NO_SUCH_FILE) {
                    sftp.mkdir(currentPath.toString());
                } else
                    throw e;
            }
        }
    }
}
