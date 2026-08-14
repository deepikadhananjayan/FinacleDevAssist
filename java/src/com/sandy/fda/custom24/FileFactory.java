package com.sandy.fda.custom24;

import java.util.HashMap;
import java.util.Map;

import com.sandy.fda.custom24.files.GincGenerator;
import com.sandy.fda.custom24.files.GlinkGenerator;
import com.sandy.fda.custom24.files.HelpGenerator;
import com.sandy.fda.custom24.files.IncGenerator;
import com.sandy.fda.custom24.files.InfengGenerator;
import com.sandy.fda.custom24.files.LinkGenerator;
import com.sandy.fda.custom24.files.PropGenerator;
import com.sandy.fda.custom24.files.SqlGenerator;
import com.sandy.fda.custom24.files.XmlGenerator;
import com.sandy.fda.models.custom24.enums.FileType;

public class FileFactory {
    private final Map<FileType, IFileGenerator> fileGenerators = new HashMap<>();

    public FileFactory() {
        fileGenerators.put(FileType.GINC, new GincGenerator());
        fileGenerators.put(FileType.INC, new IncGenerator());
        fileGenerators.put(FileType.INFENG, new InfengGenerator());
        fileGenerators.put(FileType.PROPS, new PropGenerator());
        fileGenerators.put(FileType.LINK, new LinkGenerator());
        fileGenerators.put(FileType.GLINK, new GlinkGenerator());
        fileGenerators.put(FileType.XML, new XmlGenerator());
        fileGenerators.put(FileType.HELP, new HelpGenerator());
        fileGenerators.put(FileType.SQL, new SqlGenerator());
    }

    public IFileGenerator getFileGenerator(FileType type) {
        IFileGenerator fileGenerator = fileGenerators.getOrDefault(type, null);
        if (fileGenerator == null)
            throw new IllegalArgumentException("No Generator found for type: " + type);
        return fileGenerator;
    }
}
