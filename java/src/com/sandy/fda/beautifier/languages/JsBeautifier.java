package com.sandy.fda.beautifier.languages;

import com.sandy.fda.beautifier.ICodeBeautifier;
import com.sandy.fda.models.beautifier.BeautifyData;

import io.beautifier.javascript.JavaScriptBeautifier;

public class JsBeautifier implements ICodeBeautifier {

    @Override
    public String beautify(BeautifyData beautifyData) throws Exception {
        String uglyCode = beautifyData.getContent();
        return new JavaScriptBeautifier(uglyCode).beautify();
    }
}