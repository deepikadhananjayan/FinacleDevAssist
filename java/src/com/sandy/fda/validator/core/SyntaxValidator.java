package com.sandy.fda.validator.core;

import java.util.ArrayDeque;
import java.util.List;

import com.sandy.fda.models.Issue;
import com.sandy.fda.models.Line;
import com.sandy.fda.models.ScriptInfo;
import com.sandy.fda.models.SubToken;
import com.sandy.fda.models.enums.IssueType;
import com.sandy.fda.models.enums.LineType;
import com.sandy.fda.parser.TokenParser;
import com.sandy.fda.parser.Tokenizer;
import com.sandy.fda.utils.FDAIssueMessage;
import com.sandy.fda.utils.ValidatorUtil;
import com.sandy.fda.validator.line.IfValidator;
import com.sandy.fda.validator.line.LabelValidator;
import com.sandy.fda.validator.line.UserhookValidator;
import com.sandy.fda.validator.line.WhileValidator;
import com.sandy.fda.validator.line.rules.ConditionValidator;
import com.sandy.fda.validator.line.rules.FunctionValidator;

public class SyntaxValidator {

    private static ConditionValidator conditionValidator;
    private static IfValidator ifValidator;
    private static WhileValidator whileValidator;
    private static LabelValidator labelValidator;
    private static UserhookValidator userhookValidator;
    private static FunctionValidator functionValidator;

    private static Tokenizer tokenizer;

    public SyntaxValidator(TokenParser tokenParser) {
        tokenizer = new Tokenizer(tokenParser);

        labelValidator = new LabelValidator();
        userhookValidator = new UserhookValidator(tokenParser);
        functionValidator = new FunctionValidator(tokenParser);
        conditionValidator = new ConditionValidator(functionValidator);
        ifValidator = new IfValidator(tokenParser, conditionValidator);
        whileValidator = new WhileValidator(tokenParser, conditionValidator);
    }

    @SuppressWarnings("unchecked")
    public void validate(ScriptInfo scriptInfo) throws Exception {
        List<Line> scrLines = scriptInfo.getAllLines();

        ArrayDeque<Line> structStack = new ArrayDeque<>();
        Line prevLine;

        for (Line line : scrLines) {

            List<SubToken> tokens = null;

            if (line.getType() == LineType.IF || line.getType() == LineType.WHILE) {
                Object obj = tokenizer.tokenize(line);

                if (obj instanceof Issue) {
                    Issue issue = (Issue) obj;
                    scriptInfo.getIssues().add(issue);
                    continue;
                }

                tokens = (List<SubToken>) obj;
            }

            // TODO Handle Tokenizer for All Cases (Now Handled for Condition)
            // Object obj = tokenizer.tokenize(line);

            // if (obj instanceof Issue) {
            // Issue issue = (Issue) obj;
            // scriptInfo.getIssues().add(issue);
            // continue;
            // }

            // List<Token> tokens = (List<Token>) obj;

            switch (line.getType()) {
                case IF: {
                    ifValidator.validate(line, tokens, scriptInfo);
                    structStack.push(line);
                    break;
                }
                case ELSE: {
                    ifValidator.validate(line, tokens, scriptInfo);
                    prevLine = structStack.isEmpty() ? null : structStack.peek();
                    if (prevLine.getType().equals(LineType.IF)) {
                        structStack.pop();
                        structStack.push(line);
                    }
                    break;
                }
                case ENDIF: {
                    ifValidator.validate(line, tokens, scriptInfo);
                    prevLine = structStack.isEmpty() ? null : structStack.peek();
                    if (prevLine == null) {
                        structStack.push(line);
                        break;
                    }

                    if (prevLine.getType().equals(LineType.IF) || prevLine.getType().equals(LineType.ELSE)) {
                        structStack.pop();
                    }
                    break;
                }
                case WHILE: {
                    whileValidator.validate(line, tokens, scriptInfo);
                    structStack.push(line);
                    break;
                }
                case DO: {
                    whileValidator.validate(line, tokens, scriptInfo);
                    prevLine = structStack.isEmpty() ? null : structStack.peek();
                    if (prevLine == null) {
                        structStack.push(line);
                        break;
                    }

                    if (prevLine.getType().equals(LineType.WHILE)) {
                        structStack.pop();
                    }
                    break;
                }
                case LABEL:
                case GOTO:
                case GOSUB: {
                    labelValidator.validate(line, tokens, scriptInfo);
                    break;
                }
                case USERHOOK: {
                    userhookValidator.validate(line, tokens, scriptInfo);
                    break;
                }
                case FUNCTION_CALL: {
                    functionValidator.validate(line, tokens, scriptInfo);
                    break;
                }
                case FUNCTION: {
                    structStack.push(line);
                    break;
                }
                case ENDFUNCTION: {
                    prevLine = structStack.isEmpty() ? null : structStack.peek();
                    if (prevLine == null) {
                        structStack.push(line);
                        break;
                    }

                    if (prevLine.getType().equals(LineType.FUNCTION)) {
                        structStack.pop();
                    }
                    break;
                }
                case START: {
                    if (!line.getLineContent().matches("^<--START$")) {
                        scriptInfo.getIssues().add(ValidatorUtil.buildUnexpectedTokenIssue(line));
                        return;
                    }
                    break;
                }
                case END: {
                    if (!line.getLineContent().matches("^END-->$")) {
                        scriptInfo.getIssues().add(ValidatorUtil.buildUnexpectedTokenIssue(line));
                        return;
                    }
                    break;
                }
                case TRACEON: {
                    if (!line.getLineContent().matches("^TRACE\\s+ON$")) {
                        scriptInfo.getIssues().add(ValidatorUtil.buildUnexpectedTokenIssue(line));
                        return;
                    }
                    break;
                }
                case TRACEOFF: {
                    if (!line.getLineContent().matches("^TRACE\\s+OFF$")) {
                        scriptInfo.getIssues().add(ValidatorUtil.buildUnexpectedTokenIssue(line));
                        return;
                    }
                    break;
                }
                case EXITSCRIPT: {
                    if (!line.getLineContent().matches("^EXITSCRIPT$")) {
                        scriptInfo.getIssues().add(ValidatorUtil.buildUnexpectedTokenIssue(line));
                        return;
                    }
                    break;
                }
                default:
                    break;
            }
        }

        collectStructureIssues(structStack, scriptInfo);
    }

    private void collectStructureIssues(ArrayDeque<Line> structStack, ScriptInfo scriptInfo) {
        for (Line line : structStack) {
            switch (line.getType()) {
                case IF:
                    scriptInfo.getIssues().add(new Issue.Builder()
                            .addLine(line)
                            .setIssueMessage(FDAIssueMessage.EXPECTED_ENDIF_FOR_IF + line.getLineNo())
                            .setType(IssueType.ERROR)
                            .build());
                    break;

                case ELSE:
                    scriptInfo.getIssues().add(new Issue.Builder()
                            .addLine(line)
                            .setIssueMessage(FDAIssueMessage.EXPECTED_ENDIF_FOR_ELSE + line.getLineNo())
                            .setType(IssueType.ERROR)
                            .build());
                    break;

                case ENDIF:
                    scriptInfo.getIssues().add(new Issue.Builder()
                            .addLine(line)
                            .setIssueMessage(FDAIssueMessage.UNEXPECTED_ENDIF + line.getLineNo())
                            .setType(IssueType.ERROR)
                            .build());
                    break;

                case WHILE:
                    scriptInfo.getIssues().add(new Issue.Builder()
                            .addLine(line)
                            .setIssueMessage(FDAIssueMessage.EXPECTED_DO_FOR_WHILE + line.getLineNo())
                            .setType(IssueType.ERROR)
                            .build());
                    break;

                case DO:
                    scriptInfo.getIssues().add(new Issue.Builder()
                            .addLine(line)
                            .setIssueMessage(FDAIssueMessage.UNEXPECTED_DO + line.getLineNo())
                            .setType(IssueType.ERROR)
                            .build());
                    break;

                case FUNCTION:
                    scriptInfo.getIssues().add(new Issue.Builder()
                            .addLine(line)
                            .setIssueMessage(FDAIssueMessage.EXPECTED_ENDFUNCTION + line.getLineNo())
                            .setType(IssueType.ERROR)
                            .build());
                    break;

                case ENDFUNCTION:
                    scriptInfo.getIssues().add(new Issue.Builder()
                            .addLine(line)
                            .setIssueMessage(FDAIssueMessage.UNEXPECTED_ENDFUNCTION + line.getLineNo())
                            .setType(IssueType.ERROR)
                            .build());
                    break;

                default:
                    break;
            }
        }
    }
}
