package com.sandy.fda.validator.line.rules;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.List;
import java.util.stream.Collectors;

import com.sandy.fda.models.Issue;
import com.sandy.fda.models.Line;
import com.sandy.fda.models.ScriptInfo;
import com.sandy.fda.models.SubToken;
import com.sandy.fda.models.enums.IssueType;
import com.sandy.fda.models.enums.State;
import com.sandy.fda.models.enums.TokenType;
import com.sandy.fda.utils.FDAIssueMessage;

@SuppressWarnings("null")
public class ConditionValidator {

    private FunctionValidator functionValidator;

    public ConditionValidator(FunctionValidator functionValidator) {
        this.functionValidator = functionValidator;
    }

    public void validate(Line line, List<SubToken> tokens, ScriptInfo scriptInfo) throws Exception {
        ArrayDeque<SubToken> brackets = new ArrayDeque<>();
        List<SubToken> expression = new ArrayList<>();

        for (int i = 0; i < tokens.size(); i++) {
            SubToken token = tokens.get(i);
            TokenType currType = token.getType();
            TokenType prevType = !expression.isEmpty()
                    ? expression.get(expression.size() - 1).getType()
                    : null;

            if (currType == TokenType.OPEN_BRACKET) {
                brackets.add(token);
                continue;
            }

            if (currType == TokenType.CLOSE_BRACKET) {
                if (brackets.peek() == null || brackets.peek().getType() != TokenType.OPEN_BRACKET) {
                    scriptInfo.getIssues().add(new Issue.Builder()
                            .addLine(line)
                            .setIssueMessage(
                                    FDAIssueMessage.CLOSE_BRACKET_NOT_FOUND_IN_CONDITION + line.getType() + " in "
                                            + line.getLineNo())
                            .setType(IssueType.ERROR)
                            .build());
                }
                brackets.pop();

                Issue issue = validateExpression(expression, line);

                if (issue != null) {
                    scriptInfo.getIssues().add(issue);
                    return;
                }

                continue;
            }

            if (currType == TokenType.UNKNOWN_IDENTIFIER) {
                scriptInfo.getIssues().add(new Issue.Builder()
                        .addLine(line)
                        .setType(IssueType.ERROR)
                        .setIssueMessage("[" + token.getValue() + "] "
                                + FDAIssueMessage.INVALID_IDENTIFIER_FOUND_IN_CONDITION + line.getType() + " in "
                                + line.getLineNo())
                        .build());
                return;
            }

            if (currType == TokenType.AND_OPERATOR
                    || currType == TokenType.OR_OPERATOR) {
                if (!isValue(prevType) && prevType != TokenType.VALID_EXPRESSION
                        && prevType != TokenType.VALID_FUNCTION) {
                    scriptInfo.getIssues().add(new Issue.Builder()
                            .addLine(line)
                            .setIssueMessage(
                                    FDAIssueMessage.INVALID_AND_OR_OPERATOR_IN_CONDITION + line.getType() + " in "
                                            + line.getLineNo())
                            .setType(IssueType.ERROR)
                            .build());
                    return;
                }
                expression.add(token);
                continue;
            }

            if (currType == TokenType.ARITHMETIC_OPERATOR
                    || currType == TokenType.COMPARISON_OPERATOR) {
                if (!isValue(prevType) && prevType != TokenType.VALID_EXPRESSION
                        && prevType != TokenType.VALID_FUNCTION) {
                    scriptInfo.getIssues().add(new Issue.Builder()
                            .addLine(line)
                            .setIssueMessage(FDAIssueMessage.INVALID_ARITHMETIC_COMPARISON_OPERATOR_IN_CONDITION
                                    + line.getType() + " in " + line.getLineNo())
                            .setType(IssueType.ERROR)
                            .build());
                    return;
                }
                expression.add(token);
                continue;
            }

            if (currType == TokenType.FUNCTION) {
                StringBuilder function = new StringBuilder();
                i = functionValidator.validateFunction(i, tokens, function, line, scriptInfo);

                if (i == -1) {
                    return;
                }

                SubToken validFunc = new SubToken(TokenType.VALID_FUNCTION, function.toString());
                expression.add(validFunc);
                continue;
            }

            if (currType == TokenType.COMMA) {
                scriptInfo.getIssues().add(new Issue.Builder()
                        .addLine(line)
                        .setIssueMessage(FDAIssueMessage.UNEXPECTED_COMMA_IN_CONDITION + line.getType() + " in "
                                + line.getLineNo())
                        .setType(IssueType.ERROR)
                        .build());
                return;
            }

            expression.add(token);
        }

        if (!brackets.isEmpty()) {
            scriptInfo.getIssues().add(new Issue.Builder()
                    .addLine(line)
                    .setIssueMessage(FDAIssueMessage.EXTRA_BRACKETS_FOUND_IN_CONDITION + line.getType() + " in "
                            + line.getLineNo())
                    .setType(IssueType.ERROR)
                    .build());
            return;
        }
    }

    private Issue validateExpression(List<SubToken> expression, Line line) {

        if (expression.isEmpty()) {
            return new Issue.Builder()
                    .addLine(line)
                    .setIssueMessage(
                            FDAIssueMessage.EMPTY_EXPRESSION_IN_CONDITION + line.getType() + " in " + line.getLineNo())
                    .setType(IssueType.ERROR)
                    .build();
        }

        String xpr = expression.stream()
                .map(SubToken::getValue)
                .collect(Collectors.joining(" "));

        int comparisonCount = 0;

        State state = State.EXPECT_VALUE;

        for (int i = 0; i < expression.size(); i++) {

            SubToken token = expression.get(i);

            switch (state) {

                case EXPECT_VALUE:

                    switch (token.getType()) {

                        case NUMBER_LITERAL:
                        case STRING_OR_CHAR_LITERAL:
                        case SV_VARIABLE:
                        case FV_VARIABLE:
                        case REP_VARIABLE:
                        case VALID_FUNCTION:
                        case VALID_EXPRESSION:

                            state = State.EXPECT_OPERATOR;
                            break;

                        case ARITHMETIC_OPERATOR:

                            if (!"-".equals(token.getValue())) {
                                return new Issue.Builder()
                                        .addLine(line)
                                        .setIssueMessage("[" + token.getValue() + "] " +
                                                FDAIssueMessage.EMPTY_EXPRESSION_IN_CONDITION + line.getType() + " in "
                                                + line.getLineNo())
                                        .setType(IssueType.ERROR)
                                        .build();
                            }

                            if (i + 1 >= expression.size()
                                    || expression.get(i + 1).getType() != TokenType.NUMBER_LITERAL) {

                                return new Issue.Builder()
                                        .addLine(line)
                                        .setIssueMessage(
                                                FDAIssueMessage.UNARY_MINUS_ALLOWED_FOR_NUMBER_LITERAL
                                                        + line.getLineNo())
                                        .setType(IssueType.ERROR)
                                        .build();
                            }

                            break;

                        default:
                            return new Issue.Builder()
                                    .addLine(line)
                                    .setIssueMessage(
                                            FDAIssueMessage.EXPECTED_VALUE_IN_EXPRESSION + line.getType() + " in "
                                                    + line.getLineNo())
                                    .setType(IssueType.ERROR)
                                    .build();
                    }
                    break;

                case EXPECT_OPERATOR:

                    switch (token.getType()) {

                        case ARITHMETIC_OPERATOR:
                        case AND_OPERATOR:
                        case OR_OPERATOR:
                            state = State.EXPECT_VALUE;
                            break;
                        case COMPARISON_OPERATOR:
                            comparisonCount++;
                            state = State.EXPECT_VALUE;
                            break;

                        default:
                            return new Issue.Builder()
                                    .addLine(line)
                                    .setIssueMessage(
                                            FDAIssueMessage.EXPECTED_OPERATOR_IN_EXPRESSION + line.getType() + " in "
                                                    + line.getLineNo())
                                    .setType(IssueType.ERROR)
                                    .build();
                    }
                    break;
            }
        }

        if (state == State.EXPECT_VALUE) {
            return new Issue.Builder()
                    .addLine(line)
                    .setIssueMessage(
                            FDAIssueMessage.INVALID_EXPRESSION_IN_CONDITION + line.getType() + " in "
                                    + line.getLineNo())
                    .setType(IssueType.ERROR)
                    .build();
        }

        boolean hasComparison = comparisonCount > 0;

        if (!hasComparison) {
            if (state == State.EXPECT_OPERATOR) {
                expression.clear();
                SubToken validExp = new SubToken(TokenType.VALID_EXPRESSION, xpr.toString());
                expression.add(validExp);
                return null;
            }

            return new Issue.Builder()
                    .addLine(line)
                    .setIssueMessage(
                            FDAIssueMessage.INVALID_EXPRESSION_IN_CONDITION + line.getType() + " in "
                                    + line.getLineNo())
                    .setType(IssueType.ERROR)
                    .build();
        }

        expression.clear();
        SubToken validExp = new SubToken(TokenType.VALID_EXPRESSION, xpr.toString());
        expression.add(validExp);

        return null;
    }

    private boolean isValue(TokenType type) {
        return type == TokenType.STRING_OR_CHAR_LITERAL
                || type == TokenType.NUMBER_LITERAL
                || type == TokenType.SV_VARIABLE
                || type == TokenType.FV_VARIABLE
                || type == TokenType.LV_VARIABLE
                || type == TokenType.REP_VARIABLE;
    }
}
