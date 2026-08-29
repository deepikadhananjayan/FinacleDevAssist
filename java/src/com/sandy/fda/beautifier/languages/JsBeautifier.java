package com.sandy.fda.beautifier.languages;

import com.sandy.fda.beautifier.ICodeBeautifier;
import com.sandy.fda.models.beautifier.BeautifyData;

import io.beautifier.javascript.JavaScriptBeautifier;
import io.beautifier.javascript.JavaScriptOptions;

public class JsBeautifier implements ICodeBeautifier {

    @Override
    public String beautify(BeautifyData beautifyData) throws Exception {
        String uglyCode = beautifyData.getContent();
        JavaScriptOptions options = JavaScriptOptions.builder()
                .preserve_newlines(true)
                .max_preserve_newlines(2)
                .build();
        return new JavaScriptBeautifier(uglyCode, options).beautify();
    }
}