#include "highlighter.h"


Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::darkBlue);//类型名称（数，判，法），加粗，深蓝
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "数" << "判" << "法";
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    singleLineCommentFormat.setForeground(Qt::darkGreen);//注释，绿色
    rule.pattern = QRegularExpression("注[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    numFormat.setFontItalic(true);// 数字和bool，斜体，蓝色，
    numFormat.setForeground(Qt::blue);
    QStringList numPatterns;
    numPatterns << "零" << "一" << "二" << "三" << "四"
                << "五" << "六" << "七" << "八" << "九"
                << "十" << "百" << "千" << "万" << "亿"
                << "负" << "盈" << "亏" ;
    foreach (const QString &pattern, numPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = numFormat;
        highlightingRules.append(rule);
        }

    numFormat.setForeground(Qt::darkCyan);
        QStringList loopPatterns;
        loopPatterns << "当" << "若" << "且" << "否" << "离"
                    << "止" << "继" << "乃";
        foreach (const QString &pattern, loopPatterns) {
            rule.pattern = QRegularExpression(pattern);
            rule.format = numFormat;
            highlightingRules.append(rule);
            }


    multiLineCommentFormat.setForeground(Qt::darkMagenta);// 变量名称，深紫色
    commentStartExpression = QRegularExpression("“");
    commentEndExpression = QRegularExpression("”");
}


void Highlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);


    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}
