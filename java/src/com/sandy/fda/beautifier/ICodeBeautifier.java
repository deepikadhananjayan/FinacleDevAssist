package com.sandy.fda.beautifier;

public interface ICodeBeautifier {
    String beautify(String filePath) throws Exception;

    String getExtension();
}