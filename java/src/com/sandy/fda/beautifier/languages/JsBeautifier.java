package com.sandy.fda.beautifier.languages;

import com.sandy.fda.beautifier.ICodeBeautifier;
import com.sandy.fda.utils.FDAUtils;

import io.beautifier.javascript.JavaScriptBeautifier;

public class JsBeautifier implements ICodeBeautifier {

    @Override
    public String beautify(String filePath) throws Exception {
        String uglyCode = FDAUtils.getAllLinesAsString(filePath);
        return new JavaScriptBeautifier(uglyCode).beautify();
    }

    @Override
    public String getExtension() {
        return ".js";
    }
}