package com.sandy.fda.beautifier.languages;

import java.util.List;

import com.sandy.fda.beautifier.ICodeBeautifier;
import com.sandy.fda.models.beautifier.BeautifyData;
import com.sandy.fda.models.validator.Issue;
import com.sandy.fda.models.validator.Line;
import com.sandy.fda.models.validator.SubToken;
import com.sandy.fda.models.validator.enums.LineType;
import com.sandy.fda.models.validator.enums.TokenType;
import com.sandy.fda.parser.ScriptParser;
import com.sandy.fda.parser.TokenParser;
import com.sandy.fda.parser.Tokenizer;

public class ScriptBeautifier implements ICodeBeautifier {

    private ScriptParser scriptParser;
    private Tokenizer tokenizer;

    public ScriptBeautifier(TokenParser tokenParser, Tokenizer tokenizer) {
        this.scriptParser = new ScriptParser(tokenParser);
        this.tokenizer = tokenizer;
    }

    @Override
    public String beautify(BeautifyData beautifyData) throws Exception {
        List<Line> scrLines = scriptParser.parse(beautifyData.getContent(), false);
        return beautifyIt(scrLines);
    }

    private String beautifyIt(List<Line> scrLines) throws Exception {
        int indent = 1;
        StringBuilder beautifiedCode = new StringBuilder();

        for (Line line : scrLines) {

            String lineContent = beautifyLine(line);

            switch (line.getType()) {
                case START:
                case TRACEON:
                case EXITSCRIPT:
                case TRACEOFF:
                case END:
                    addIndents(lineContent, beautifiedCode, 0);
                    break;
                case EMPTYLINE:
                    addEmptyLineIfNeeded(beautifiedCode);
                    break;
                case IF:
                case WHILE:
                    addEmptyLineIfNeeded(beautifiedCode);
                    addIndents(lineContent, beautifiedCode, indent);
                    indent++;
                    break;
                case ELSE:
                    addIndents(lineContent, beautifiedCode, indent - 1);
                    break;
                case ENDIF:
                case DO:
                    indent--;
                    addIndents(lineContent, beautifiedCode, indent);
                    addEmptyLineIfNeeded(beautifiedCode);
                    break;
                case BLOCK:
                    indent--;
                    addIndents(lineContent, beautifiedCode, indent);
                    indent++;
                    break;
                case LABEL:
                    addEmptyLineIfNeeded(beautifiedCode);
                    addIndents(lineContent, beautifiedCode, indent);
                    addEmptyLineIfNeeded(beautifiedCode);
                    break;
                default:
                    addIndents(lineContent, beautifiedCode, indent);
                    break;
            }
        }

        return beautifiedCode.toString();
    }

    @SuppressWarnings("unchecked")
    private String beautifyLine(Line line) throws Exception {

        if (line.getType() == LineType.COMMENTLINE) {
            return line.getLineContent().trim();
        }

        StringBuilder lineContent = new StringBuilder();
        List<SubToken> tokens = null;
        Object obj = tokenizer.tokenize(line);

        if (!(obj instanceof Issue)) {
            tokens = (List<SubToken>) obj;

            for (SubToken subToken : tokens) {
                TokenType type = subToken.getType();

                if (type == TokenType.COMPARISON_OPERATOR
                        || type == TokenType.ASSIGNMENT_OPERATOR
                        || type == TokenType.ARITHMETIC_OPERATOR
                        || type == TokenType.AND_OPERATOR
                        || type == TokenType.OR_OPERATOR) {
                    lineContent.append(' ').append(subToken.getValue()).append(' ');
                } else if (type == TokenType.IMPORT
                        || type == TokenType.LIBNAME
                        || type == TokenType.GOTO
                        || type == TokenType.GOSUB) {
                    lineContent.append(subToken.getValue()).append(' ');
                } else {
                    lineContent.append(subToken.getValue());
                }
            }

            return lineContent.toString();
        } else {
            return line.getLineContent().trim();
        }
    }

    private void addIndents(String line, StringBuilder beautifiedCode, int indent) {
        String tabSpace = "\t";
        beautifiedCode.append(tabSpace.repeat(indent));
        beautifiedCode.append(line).append(System.lineSeparator());
    }

    private void addEmptyLineIfNeeded(StringBuilder builder) {
        String lineSeparator = System.lineSeparator();

        int length = builder.length();

        if (length >= lineSeparator.length() * 2) {
            String lastTwo = builder.substring(
                    length - (lineSeparator.length() * 2));

            if (lastTwo.equals(lineSeparator + lineSeparator)) {
                return;
            }
        }

        builder.append(lineSeparator);
    }
}
