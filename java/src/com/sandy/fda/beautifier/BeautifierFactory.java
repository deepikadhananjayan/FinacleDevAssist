package com.sandy.fda.beautifier;

import java.util.HashMap;
import java.util.Map;

import com.sandy.fda.beautifier.languages.JavaBeautifier;
import com.sandy.fda.beautifier.languages.JsBeautifier;
import com.sandy.fda.beautifier.languages.ScriptBeautifier;
import com.sandy.fda.beautifier.languages.XmlBeautifier;
import com.sandy.fda.models.beautifier.BeautifyData;
import com.sandy.fda.models.beautifier.enums.ContentType;
import com.sandy.fda.parser.TokenParser;
import com.sandy.fda.parser.Tokenizer;

public class BeautifierFactory {
    private final Map<ContentType, ICodeBeautifier> formatters = new HashMap<>();

    public BeautifierFactory(TokenParser tokenParser, Tokenizer tokenizer) {
        try {
            formatters.put(ContentType.JS, new JsBeautifier());
            formatters.put(ContentType.JAVA, new JavaBeautifier());
            formatters.put(ContentType.XML, new XmlBeautifier());
            formatters.put(ContentType.SCRIPT, new ScriptBeautifier(tokenParser, tokenizer));
        } catch (Exception e) {
            throw new RuntimeException("Failed to initialize formatters", e);
        }
    }

    public ICodeBeautifier getFormatter(BeautifyData beautifyData) {
        ICodeBeautifier beautifier = formatters.get(beautifyData.getContentType());
        if (beautifier == null)
            throw new IllegalArgumentException("No formatter found for type: " + beautifyData.getContentType());

        return beautifier;
    }
}
