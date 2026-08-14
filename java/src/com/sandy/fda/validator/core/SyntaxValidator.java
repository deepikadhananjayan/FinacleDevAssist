package com.sandy.fda.validator.core;

import java.util.ArrayDeque;
import java.util.List;
import java.util.stream.Collectors;

import com.sandy.fda.models.validator.Issue;
import com.sandy.fda.models.validator.Line;
import com.sandy.fda.models.validator.ScriptInfo;
import com.sandy.fda.models.validator.SubToken;
import com.sandy.fda.models.validator.enums.IssueType;
import com.sandy.fda.models.validator.enums.LineType;
import com.sandy.fda.models.validator.enums.TokenType;
import com.sandy.fda.parser.TokenParser;
import com.sandy.fda.parser.Tokenizer;
import com.sandy.fda.utils.FDAIssueMessage;
import com.sandy.fda.utils.FDALogger;
import com.sandy.fda.utils.FDAUtils;
import com.sandy.fda.validator.line.IfValidator;
import com.sandy.fda.validator.line.LabelValidator;
import com.sandy.fda.validator.line.UserhookValidator;
import com.sandy.fda.validator.line.WhileValidator;
import com.sandy.fda.validator.line.rules.ConditionValidator;
import com.sandy.fda.validator.line.rules.FunctionValidator;

public class SyntaxValidator {

    private Tokenizer tokenizer;
    private ConditionValidator conditionValidator;
    private IfValidator ifValidator;
    private WhileValidator whileValidator;
    private LabelValidator labelValidator;
    private UserhookValidator userhookValidator;
    private FunctionValidator functionValidator;

    public SyntaxValidator(TokenParser tokenParser, Tokenizer tokenizer) {
        this.tokenizer = tokenizer;

        this.labelValidator = new LabelValidator(tokenizer);
        this.userhookValidator = new UserhookValidator(tokenParser);
        this.functionValidator = new FunctionValidator(tokenParser);
        this.conditionValidator = new ConditionValidator(functionValidator);
        this.ifValidator = new IfValidator(tokenParser, conditionValidator);
        this.whileValidator = new WhileValidator(tokenParser, conditionValidator);
    }

    @SuppressWarnings({ "unchecked", "null" })
    public void validate(ScriptInfo scriptInfo) throws Exception {
        List<Line> scrLines = scriptInfo.getAllLines();

        ArrayDeque<Line> structStack = new ArrayDeque<>();
        Line prevLine;

        for (Line line : scrLines) {

            List<SubToken> tokens = null;
            Issue issue = null;

            if (line.getType() == LineType.EMPTYLINE || line.getType() == LineType.BLOCK) {
                continue;
            }

            Object obj = tokenizer.tokenize(line);

            if (obj instanceof Issue)
                issue = (Issue) obj;
            else {
                tokens = (List<SubToken>) obj;
                rmvCmtFromTokens(tokens);
            }

            switch (line.getType()) {
                case IF: {
                    structStack.push(line);

                    if (issue != null) {
                        scriptInfo.getIssues().add(issue);
                        continue;
                    }

                    ifValidator.validate(line, tokens, scriptInfo);
                    break;
                }
                case ELSE: {
                    prevLine = structStack.isEmpty() ? null : structStack.peek();
                    if (prevLine == null) {
                        structStack.push(line);
                        break;
                    }

                    if (prevLine.getType().equals(LineType.IF)) {
                        structStack.pop();
                        structStack.push(line);
                    }

                    if (issue != null) {
                        scriptInfo.getIssues().add(issue);
                        continue;
                    }

                    ifValidator.validate(line, tokens, scriptInfo);
                    break;
                }
                case ENDIF: {
                    prevLine = structStack.isEmpty() ? null : structStack.peek();
                    if (prevLine == null) {
                        structStack.push(line);
                        break;
                    }

                    if (prevLine.getType().equals(LineType.IF) || prevLine.getType().equals(LineType.ELSE)) {
                        structStack.pop();
                    }

                    if (issue != null) {
                        scriptInfo.getIssues().add(issue);
                        continue;
                    }

                    ifValidator.validate(line, tokens, scriptInfo);
                    break;
                }
                case WHILE: {
                    structStack.push(line);

                    if (issue != null) {
                        scriptInfo.getIssues().add(issue);
                        continue;
                    }

                    whileValidator.validate(line, tokens, scriptInfo);
                    break;
                }
                case DO: {
                    prevLine = structStack.isEmpty() ? null : structStack.peek();
                    if (prevLine == null) {
                        structStack.push(line);
                        break;
                    }

                    if (prevLine.getType().equals(LineType.WHILE)) {
                        structStack.pop();
                    }

                    if (issue != null) {
                        scriptInfo.getIssues().add(issue);
                        continue;
                    }

                    whileValidator.validate(line, tokens, scriptInfo);
                    break;
                }
                case LABEL:
                case GOTO:
                case GOSUB: {
                    if (issue != null) {
                        scriptInfo.getIssues().add(issue);
                        continue;
                    }
                    
                    labelValidator.validate(line, tokens, scriptInfo);
                    break;
                }
                case USERHOOK: {
                    if (issue != null) {
                        scriptInfo.getIssues().add(issue);
                        continue;
                    }
                    
                    userhookValidator.validate(line, tokens, scriptInfo);
                    break;
                }
                case FUNCTION_CALL: {
                    if (issue != null) {
                        scriptInfo.getIssues().add(issue);
                        continue;
                    }

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
                case LIBNAME: {
                    if (issue != null) {
                        scriptInfo.getIssues().add(issue);
                        break;
                    }

                    if (tokens != null) {
                        FDALogger.info(tokens.stream().map(SubToken::getValue)
                                .collect(Collectors.joining(" ")));
                        if (tokens.size() > 2) {
                            scriptInfo.getIssues().add(FDAUtils.buildUnexpectedTokenIssue(line));
                        } else if (tokens.size() == 1) {
                            scriptInfo.getIssues().add(new Issue.Builder()
                                    .addLine(line)
                                    .setType(IssueType.ERROR)
                                    .setIssueMessage(FDAIssueMessage.LIBNAME_UNDEFINED + line.getLineNo()).build());
                        } else if (!tokens.get(1).getValue().equals("CUSTOMSO")) {
                            scriptInfo.getIssues().add(new Issue.Builder()
                                    .addLine(line)
                                    .setType(IssueType.WARNING)
                                    .setIssueMessage(FDAIssueMessage.LIBNAME_NOT_REGISTERED + line.getLineNo())
                                    .build());
                        }
                    }
                    break;
                }
                case IMPORT: {
                    if (issue != null) {
                        scriptInfo.getIssues().add(issue);
                        break;
                    }

                    if (tokens != null) {
                        if (tokens.size() > 2) {
                            scriptInfo.getIssues().add(FDAUtils.buildUnexpectedTokenIssue(line));
                        } else if (tokens.size() == 1) {
                            scriptInfo.getIssues().add(new Issue.Builder()
                                    .addLine(line)
                                    .setType(IssueType.ERROR)
                                    .setIssueMessage(FDAIssueMessage.IMPORT_UNDEFINED + line.getLineNo()).build());
                        }
                    }
                    break;
                }
                case ASSIGNMENT:{
                    //FDALogger.info("Line No " + line.getLineNo() + " Line Type -> " + line.getType());
                    // if (issue != null) {
                    //     scriptInfo.getIssues().add(issue);
                    //     FDALogger.info(issue.getIssueMessage());
                    //     break;
                    // }

                    //FDALogger.info(tokens.stream().map(SubToken::toString).collect(Collectors.joining(" ")));
                    //Assignment Logic
                    break;
                }
                case DYNAMIC_ASSIGNMENT:{
                    break;
                }
                case START:
                case END:
                case TRACEON:
                case TRACEOFF:
                case EXITSCRIPT: {
                    if (issue != null) {
                        scriptInfo.getIssues().add(issue);
                        break;
                    }

                    if (tokens != null) {
                        if (tokens.size() > 1) {
                            scriptInfo.getIssues().add(FDAUtils.buildUnexpectedTokenIssue(line));
                        }
                    }
                    break;
                }
                case EMPTYLINE:
                case COMMENTLINE:
                case BLOCK:
                    break;
                default:
                    // FDALogger.info("Line No " + line.getLineNo() + " Line Type -> " + line.getType());

                    // if (issue != null) {
                    //     scriptInfo.getIssues().add(issue);
                    //     FDALogger.info(issue.getIssueMessage());
                    //     break;
                    // }

                    // FDALogger.info(tokens.stream().map(SubToken::toString).collect(Collectors.joining(" ")));
                    // break;
            }
        }

        collectStructureIssues(structStack, scriptInfo);
    }

    private void rmvCmtFromTokens(List<SubToken> tokens) {
        if (tokens == null) {
            return;
        }

        int lastIdx = tokens.size() - 1;

        if (lastIdx != -1 && tokens.get(lastIdx).getType() == TokenType.COMMENT_CONTENT) {
            tokens.remove(lastIdx);
        }
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
