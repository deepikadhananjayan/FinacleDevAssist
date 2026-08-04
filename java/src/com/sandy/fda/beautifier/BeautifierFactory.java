package com.sandy.fda.beautifier;

import java.util.HashMap;
import java.util.Map;

import com.sandy.fda.beautifier.languages.JavaBeautifier;
import com.sandy.fda.beautifier.languages.JsBeautifier;
import com.sandy.fda.beautifier.languages.ScriptBeautifier;
import com.sandy.fda.parser.TokenParser;
import com.sandy.fda.parser.Tokenizer;

public class BeautifierFactory {
    private final Map<String, ICodeBeautifier> formatters = new HashMap<>();

    public BeautifierFactory(TokenParser tokenParser, Tokenizer tokenizer) {
        try {
            formatters.put(".js", new JsBeautifier());
            formatters.put(".java", new JavaBeautifier());
            formatters.put(".scr", new ScriptBeautifier(tokenParser, tokenizer));
        } catch (Exception e) {
            throw new RuntimeException("Failed to initialize formatters", e);
        }
    }

    public ICodeBeautifier getFormatter(String fileExtension) {
        ICodeBeautifier beautifier = formatters.get(fileExtension.toLowerCase());
        if (beautifier == null)
            throw new IllegalArgumentException("No formatter found for extension: " + fileExtension);

        return beautifier;
    }

    public ICodeBeautifier getFormatterForFile(String filename) {
        int lastDot = filename.lastIndexOf('.');
        if (lastDot == -1)
            throw new IllegalArgumentException("No extension found");
        return getFormatter(filename.substring(lastDot));
    }
}
