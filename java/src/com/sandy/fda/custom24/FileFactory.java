package com.sandy.fda.custom24;

import java.util.HashMap;
import java.util.Map;

import com.sandy.fda.beautifier.Beautifier;
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
    private TemplateService templateService;

    public FileFactory(Beautifier beautifier) {
        this.templateService = new TemplateService(beautifier);
        fileGenerators.put(FileType.GINC, new GincGenerator(templateService));
        fileGenerators.put(FileType.INC, new IncGenerator(templateService));
        fileGenerators.put(FileType.INFENG, new InfengGenerator(templateService));
        fileGenerators.put(FileType.PROPS, new PropGenerator(templateService));
        fileGenerators.put(FileType.LINK, new LinkGenerator(templateService));
        fileGenerators.put(FileType.GLINK, new GlinkGenerator(templateService));
        fileGenerators.put(FileType.XML, new XmlGenerator(templateService));
        fileGenerators.put(FileType.HELP, new HelpGenerator(templateService));
        fileGenerators.put(FileType.SQL, new SqlGenerator(templateService));
    }

    public IFileGenerator getFileGenerator(FileType type) {
        IFileGenerator fileGenerator = fileGenerators.getOrDefault(type, null);
        if (fileGenerator == null)
            throw new IllegalArgumentException("No Generator found for type: " + type);
        return fileGenerator;
    }
}
