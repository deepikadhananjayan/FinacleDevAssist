package com.sandy.fda.beautifier.languages;

import com.google.googlejavaformat.java.Formatter;
import com.sandy.fda.beautifier.ICodeBeautifier;
import com.sandy.fda.utils.FDAUtils;

public class JavaBeautifier implements ICodeBeautifier {
    @Override
    public String beautify(String filePath) throws Exception {
        String uglyCode = FDAUtils.getAllLinesAsString(filePath);
        return new Formatter().formatSourceAndFixImports(uglyCode);
    }

    @Override
    public String getExtension() {
        return ".java";
    }
}
