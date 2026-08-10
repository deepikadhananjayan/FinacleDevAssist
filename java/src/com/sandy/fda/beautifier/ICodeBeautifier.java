package com.sandy.fda.beautifier;

import com.sandy.fda.models.beautifier.BeautifyData;

public interface ICodeBeautifier {
    String beautify(BeautifyData beautifyData) throws Exception;
}