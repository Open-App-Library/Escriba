#include "escriba-textedit.h"
#include <QTextDocument>
#include <QTextCursor>
#include <QImage>
#include <QByteArray>
#include <QBuffer>
#include <stdlib.h>
#include <QKeyEvent>
#include <QDebug>

Escriba_TextEdit::Escriba_TextEdit(QWidget *parent) : QTextEdit(parent) {
}


bool Escriba_TextEdit::canInsertFromMimeData(const QMimeData *source) const {
    return source->hasImage() || QTextEdit::canInsertFromMimeData(source);
}


void Escriba_TextEdit::insertFromMimeData(const QMimeData *source) {
    if (source->hasImage()) {
        QStringList formats = source->formats();
        QString format;
        for (int i=0; i<formats.size(); i++) {
            if (formats[i] == "image/bmp")  { format = "BMP";  break; }
            if (formats[i] == "image/jpeg") { format = "JPG";  break; }
            if (formats[i] == "image/jpg")  { format = "JPG";  break; }
            if (formats[i] == "image/gif")  { format = "GIF";  break; }
            if (formats[i] == "image/png")  { format = "PNG";  break; }
            if (formats[i] == "image/pbm")  { format = "PBM";  break; }
            if (formats[i] == "image/pgm")  { format = "PGM";  break; }
            if (formats[i] == "image/ppm")  { format = "PPM";  break; }
            if (formats[i] == "image/tiff") { format = "TIFF"; break; }
            if (formats[i] == "image/xbm")  { format = "XBM";  break; }
            if (formats[i] == "image/xpm")  { format = "XPM";  break; }
            }
        if (!format.isEmpty()) {
//          dropImage(qvariant_cast<QImage>(source->imageData()), format);
            dropImage(qvariant_cast<QImage>(source->imageData()), "JPG"); // Sorry, ale cokoli jiného dlouho trvá
            return;
            }
        }
    QTextEdit::insertFromMimeData(source);
}


QMimeData *Escriba_TextEdit::createMimeDataFromSelection() const {
    return QTextEdit::createMimeDataFromSelection();
}

void Escriba_TextEdit::keyPressEvent(QKeyEvent *event)
{
    // Do regular key press event
    QTextEdit::keyPressEvent(event);

    // Custom
    /// Set the current line after hitting return key to blank formatting
    if(event->key() == Qt::Key_Return) {
        QTextCursor cursor = this->textCursor();
        QTextBlockFormat blockFormat = cursor.blockFormat();
        cursor.beginEditBlock();

        // standard
        if (!cursor.hasSelection()) {
            cursor.select(QTextCursor::BlockUnderCursor);
        }

        QTextCharFormat fmt;
        m_curlineformat = fmt;
        cursor.setCharFormat(fmt);
        this->setCurrentCharFormat(fmt);

        // Wrap things up
        cursor.setCharFormat(fmt);
        cursor.setBlockCharFormat(fmt);
        this->setCurrentCharFormat(fmt);
        cursor.endEditBlock();
    }
}

QTextCharFormat Escriba_TextEdit::curlineformat() const
{
    return m_curlineformat;
}

void Escriba_TextEdit::setCurlineformat(const QTextCharFormat &curlineformat)
{
    m_curlineformat = curlineformat;
}

void Escriba_TextEdit::setHtml(const QString html)
{
	QString sanitized_html = html;
	sanitized_html.prepend("<style>pre { background: #474747; margin: 20px 10px; font-family: monospace; color: white;}</style>");

	// Qt's HTML standard does not support <del>.
	// Instead they support <s>. Do a replace!
	sanitized_html.replace( "<del>", "<s>" );
	sanitized_html.replace( "</del>", "</s>" );

	sanitized_html.replace(QRegExp("[\\s\n\\n]+</code>"), "</code>");

	// Add a <p> if a </pre> is the last element in document.
	// This allows the user to not...be stuck forever in a code block.
	sanitized_html.replace(QRegExp("</pre>[\n]?$"), "</pre><p></p>");

	QTextEdit::setHtml( sanitized_html );
}

void Escriba_TextEdit::dropImage(const QImage& image, const QString& format) {
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, format.toLocal8Bit().data());
    buffer.close();
    QByteArray base64 = bytes.toBase64();
    QByteArray base64l;
    for (int i=0; i<base64.size(); i++) {
        base64l.append(base64[i]);
        if (i%80 == 0) {
            base64l.append("\n");
            }
        }

    QTextCursor cursor = textCursor();
    QTextImageFormat imageFormat;
    imageFormat.setWidth  ( image.width() );
    imageFormat.setHeight ( image.height() );
    imageFormat.setName   ( QString("data:image/%1;base64,%2")
                                .arg(QString("%1.%2").arg(rand()).arg(format))
                                .arg(base64l.data())
                                );
    cursor.insertImage    ( imageFormat );
}
