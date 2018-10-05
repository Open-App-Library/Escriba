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

  protected:
    bool        canInsertFromMimeData(const QMimeData *source) const;
    void        insertFromMimeData(const QMimeData *source);
    QMimeData  *createMimeDataFromSelection() const;

};

#endif
