#ifndef EDOCUMENT_H
#define EDOCUMENT_H
#include <QString>

class EDocument {
public:
    EDocument(QString initialMarkdown);

    void loadHTML(QString HTML);
    void loadMarkdown(QString markdown);
private:
    QString m_markdown;
    QString m_html;
};

#endif
