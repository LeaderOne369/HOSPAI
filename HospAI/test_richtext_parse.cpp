#include <QCoreApplication>
#include <QString>
#include <QRegularExpression>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // 模拟从用户消息中看到的富文本内容
    QString testMessage = R"([RICH_TEXT]￼[/RICH_TEXT][HTML]<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/TR/REC-html40/strict.dtd">
<html><head><meta name="qrichtext" content="1" /><meta charset="utf-8" /><style type="text/css">
p, li { white-space: pre-wrap; }
hr { height: 1px; border-width: 0; }
li.unchecked::marker { content: "\2610"; }
li.checked::marker { content: "\2612"; }
</style></head><body style=" font-family:'Ubuntu'; font-size:11pt; font-weight:400; font-style:normal;">
<p style=" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;"><img src="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNkYPhfDwAChwGA60e6kgAAAABJRU5ErkJggg==" /></p></body></html>[/HTML])";
    
    qDebug() << "测试消息长度:" << testMessage.length();
    
    // 测试正则表达式
    QRegularExpression richTextRegex("\\[RICH_TEXT\\](.*?)\\[/RICH_TEXT\\]", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch richTextMatch = richTextRegex.match(testMessage);
    
    QRegularExpression htmlRegex("\\[HTML\\](.*?)\\[/HTML\\]", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch htmlMatch = htmlRegex.match(testMessage);
    
    qDebug() << "富文本匹配成功:" << richTextMatch.hasMatch();
    qDebug() << "HTML匹配成功:" << htmlMatch.hasMatch();
    
    if (richTextMatch.hasMatch()) {
        QString plainText = richTextMatch.captured(1);
        qDebug() << "解析到的纯文本:" << plainText;
    }
    
    if (htmlMatch.hasMatch()) {
        QString htmlContent = htmlMatch.captured(1);
        qDebug() << "解析到的HTML长度:" << htmlContent.length();
        qDebug() << "HTML前100字符:" << htmlContent.left(100);
        
        // 检查是否包含完整HTML文档
        if (htmlContent.contains("<!DOCTYPE") && htmlContent.contains("<body")) {
            QRegularExpression bodyRegex("<body[^>]*>(.*?)</body>", QRegularExpression::DotMatchesEverythingOption);
            QRegularExpressionMatch bodyMatch = bodyRegex.match(htmlContent);
            if (bodyMatch.hasMatch()) {
                QString bodyContent = bodyMatch.captured(1);
                qDebug() << "提取的body内容长度:" << bodyContent.length();
                qDebug() << "body前100字符:" << bodyContent.left(100);
            }
        }
    }
    
    return 0;
} 