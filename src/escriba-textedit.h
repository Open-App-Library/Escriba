#ifndef ESCRIBA_TEXTEDIT_H
#define ESCRIBA_TEXTEDIT_H

#include <QTextEdit>
#include <QMimeData>
#include <QImage>

class Escriba_TextEdit : public QTextEdit {
    Q_OBJECT
  public:
    Escriba_TextEdit(QWidget *parent);

    void        dropImage(const QImage& image, const QString& format);

    QTextCharFormat curlineformat() const;
    void setCurlineformat(const QTextCharFormat &curlineformat);

protected:
    bool        canInsertFromMimeData(const QMimeData *source) const;
    void        insertFromMimeData(const QMimeData *source);
    QMimeData  *createMimeDataFromSelection() const;

    void keyPressEvent(QKeyEvent *event);

private:
    QTextCharFormat m_curlineformat;

};

#endif
