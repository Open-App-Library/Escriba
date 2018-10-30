#include "escribahelper.h"
#include <QRegExp>
#include <QStringList>

QString EscribaHelper::getInnerHtml(QString str)
{
    QRegExp rx = QRegExp("<body[^>]*>((.|[\n\r])*)<\\/body>");
    rx.indexIn(str);
    QStringList qsl = rx.capturedTexts();

    if ( qsl[1].isEmpty() )
        return str;
    return qsl[1];
}
