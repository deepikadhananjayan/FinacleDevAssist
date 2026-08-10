package com.sandy.fda.beautifier.languages;

import com.google.googlejavaformat.java.Formatter;
import com.sandy.fda.beautifier.ICodeBeautifier;
import com.sandy.fda.models.beautifier.BeautifyData;

public class JavaBeautifier implements ICodeBeautifier {
    @Override
    public String beautify(BeautifyData beautifyData) throws Exception {
        String uglyCode = beautifyData.getContent();
        return new Formatter().formatSourceAndFixImports(uglyCode);
    }
}
