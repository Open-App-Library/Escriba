/*
** Copyright (C) 2013 Jiří Procházka (Hobrasoft)
** Contact: http://www.hobrasoft.cz/
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file is under the terms of the GNU Lesser General Public License
** version 2.1 as published by the Free Software Foundation and appearing
** in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the
** GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
*/

#include "escriba.h"
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QFontDatabase>
#include <QInputDialog>
#include <QColorDialog>
#include <QTextList>
#include <QtDebug>
#include <QFileDialog>
#include <QImageReader>
#include <QSettings>
#include <QBuffer>
#include <QUrl>
#include <QPlainTextEdit>
#include <QMenu>
#include <QDialog>
#include <QTextDocumentFragment>
#include <markdownpanda/qt.hpp>
#include <qbasichtmlexporter.h>

Escriba::Escriba(QWidget *parent) :
    QWidget(parent),
    m_mdpanda(new MarkdownPandaQt())
{
    setupUi(this);
    m_lastBlockList = nullptr;

    f_richTextEdit->setTabStopDistance(40);

    m_markdownSyntax = new MarkdownSyntax( f_plainTextEdit->document() );

    connect(f_richTextEdit, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
            this,     SLOT(slotCurrentCharFormatChanged(QTextCharFormat)));
    connect(f_richTextEdit, SIGNAL(cursorPositionChanged()),
            this,     SLOT(slotCursorPositionChanged()));

    m_fontsize_h1 = 18;
    m_fontsize_h2 = 16;
    m_fontsize_h3 = 14;
    m_fontsize_h4 = 12;

    fontChanged(f_richTextEdit->font());
    bgColorChanged(f_richTextEdit->textColor());

    // paragraph formatting

    m_paragraphItems    << tr("Standard")
                        << tr("Heading 1")
                        << tr("Heading 2")
                        << tr("Heading 3")
                        << tr("Heading 4")
                        << tr("Monospace");
    f_paragraph->addItems(m_paragraphItems);

    connect(f_paragraph, SIGNAL(activated(int)),
            this, SLOT(textStyle(int)));

    // undo & redo

    f_undo->setShortcut(QKeySequence::Undo);
    f_redo->setShortcut(QKeySequence::Redo);

    connect(f_richTextEdit->document(), SIGNAL(undoAvailable(bool)),
            f_undo, SLOT(setEnabled(bool)));
    connect(f_richTextEdit->document(), SIGNAL(redoAvailable(bool)),
            f_redo, SLOT(setEnabled(bool)));

    f_undo->setEnabled(f_richTextEdit->document()->isUndoAvailable());
    f_redo->setEnabled(f_richTextEdit->document()->isRedoAvailable());

    connect(f_undo, SIGNAL(clicked()), f_richTextEdit, SLOT(undo()));
    connect(f_redo, SIGNAL(clicked()), f_richTextEdit, SLOT(redo()));

    // cut, copy & paste

    //f_cut->setShortcut(QKeySequence::Cut);
    //f_copy->setShortcut(QKeySequence::Copy);
    //f_paste->setShortcut(QKeySequence::Paste);

    //f_cut->setEnabled(false);
    //f_copy->setEnabled(false);

    //connect(f_cut, SIGNAL(clicked()), f_richTextEdit, SLOT(cut()));
    //connect(f_copy, SIGNAL(clicked()), f_richTextEdit, SLOT(copy()));
    //connect(f_paste, SIGNAL(clicked()), f_richTextEdit, SLOT(paste()));

    //connect(f_richTextEdit, SIGNAL(copyAvailable(bool)), f_cut, SLOT(setEnabled(bool)));
    //connect(f_richTextEdit, SIGNAL(copyAvailable(bool)), f_copy, SLOT(setEnabled(bool)));

    connect(f_editorTypeTab, SIGNAL(currentChanged(int)),
            this, SLOT(switchedEditorType(int)));

#ifndef QT_NO_CLIPBOARD
    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(slotClipboardDataChanged()));
#endif

    // link

    f_link->setShortcut(Qt::CTRL + Qt::Key_L);

    connect(f_link, SIGNAL(clicked(bool)), this, SLOT(textLink(bool)));

    // bold, italic & underline

    f_bold->setShortcut(Qt::CTRL + Qt::Key_B);
    f_italic->setShortcut(Qt::CTRL + Qt::Key_I);
    f_underline->setShortcut(Qt::CTRL + Qt::Key_U);

    connect(f_bold, SIGNAL(clicked()), this, SLOT(textBold()));
    connect(f_italic, SIGNAL(clicked()), this, SLOT(textItalic()));
    connect(f_underline, SIGNAL(clicked()), this, SLOT(textUnderline()));
    connect(f_strikeout, SIGNAL(clicked()), this, SLOT(textStrikeout()));

    QAction *removeFormat = new QAction(tr("Remove character formatting"), this);
    removeFormat->setShortcut(QKeySequence("CTRL+M"));
    connect(removeFormat, SIGNAL(triggered()), this, SLOT(textRemoveFormat()));
    f_richTextEdit->addAction(removeFormat);

    QAction *removeAllFormat = new QAction(tr("Remove all formatting"), this);
    connect(removeAllFormat, SIGNAL(triggered()), this, SLOT(textRemoveAllFormat()));
    f_richTextEdit->addAction(removeAllFormat);

    QAction *textsource = new QAction(tr("Edit document source"), this);
    textsource->setShortcut(QKeySequence("CTRL+O"));
    connect(textsource, SIGNAL(triggered()), this, SLOT(textSource()));
    f_richTextEdit->addAction(textsource);

    QMenu *menu = new QMenu(this);
    menu->addAction(removeAllFormat);
    menu->addAction(removeFormat);
    menu->addAction(textsource);
    f_menu->setMenu(menu);
    f_menu->setPopupMode(QToolButton::InstantPopup);

    // lists

    f_list_bullet->setShortcut(Qt::CTRL + Qt::Key_Minus);
    f_list_ordered->setShortcut(Qt::CTRL + Qt::Key_Equal);

    connect(f_list_bullet, SIGNAL(clicked(bool)), this, SLOT(listBullet(bool)));
    connect(f_list_ordered, SIGNAL(clicked(bool)), this, SLOT(listOrdered(bool)));

    // indentation

    //    f_indent_dec->setShortcut(Qt::CTRL + Qt::Key_Comma);
    //    f_indent_inc->setShortcut(Qt::CTRL + Qt::Key_Period);

    //    connect(f_indent_inc, SIGNAL(clicked()), this, SLOT(increaseIndentation()));
    //    connect(f_indent_dec, SIGNAL(clicked()), this, SLOT(decreaseIndentation()));

    // font size

    QFontDatabase db;
    //    foreach(int size, db.standardSizes())
    //        f_fontsize->addItem(QString::number(size));

    //    connect(f_fontsize, SIGNAL(activated(QString)),
    //            this, SLOT(textSize(QString)));
    //    f_fontsize->setCurrentIndex(f_fontsize->findText(QString::number(QApplication::font()
    //                                                                   .pointSize())));

    // text foreground color

    QPixmap pix(16, 16);
    pix.fill(QApplication::palette().foreground().color());
    //    f_fgcolor->setIcon(pix);

    //    connect(f_fgcolor, SIGNAL(clicked()), this, SLOT(textFgColor()));

    // text background color

    pix.fill(QApplication::palette().background().color());
    //    f_bgcolor->setIcon(pix);

    //    connect(f_bgcolor, SIGNAL(clicked()), this, SLOT(textBgColor()));

    // images
    connect(f_image, SIGNAL(clicked()), this, SLOT(insertImage()));
}


void Escriba::textSource() {
    QDialog *dialog = new QDialog(this);
    QPlainTextEdit *pte = new QPlainTextEdit(dialog);
    pte->setPlainText( QBasicHtmlExporter(f_richTextEdit->document()).toHtml() );
    QGridLayout *gl = new QGridLayout(dialog);
    gl->addWidget(pte,0,0,1,1);
    dialog->setWindowTitle(tr("Document source"));
    dialog->setMinimumWidth (400);
    dialog->setMinimumHeight(600);
    dialog->exec();

    f_richTextEdit->setHtml(pte->toPlainText());

    delete dialog;
}


void Escriba::textRemoveFormat() {
    QTextCharFormat fmt;
    fmt.setFontWeight(QFont::Normal);
    fmt.setFontUnderline  (false);
    fmt.setFontStrikeOut  (false);
    fmt.setFontItalic     (false);
    fmt.setFontPointSize  (9);
    //  fmt.setFontFamily     ("Helvetica");
    //  fmt.setFontStyleHint  (QFont::SansSerif);
    //  fmt.setFontFixedPitch (true);

    f_bold      ->setChecked(false);
    f_underline ->setChecked(false);
    f_italic    ->setChecked(false);
    f_strikeout ->setChecked(false);
    //    f_fontsize  ->setCurrentIndex(f_fontsize->findText("9"));

    //  QTextBlockFormat bfmt = cursor.blockFormat();
    //  bfmt->setIndent(0);

    fmt.clearBackground();

    mergeFormatOnWordOrSelection(fmt);
}


void Escriba::textRemoveAllFormat() {
    f_bold      ->setChecked(false);
    f_underline ->setChecked(false);
    f_italic    ->setChecked(false);
    f_strikeout ->setChecked(false);
    //    f_fontsize  ->setCurrentIndex(f_fontsize->findText("9"));
    QString text = f_richTextEdit->toPlainText();
    f_richTextEdit->setPlainText(text);
}


void Escriba::textBold() {
    QTextCharFormat fmt;
    fmt.setFontWeight(f_bold->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}


void Escriba::focusInEvent(QFocusEvent *) {
    f_richTextEdit->setFocus(Qt::TabFocusReason);
}


void Escriba::textUnderline() {
    QTextCharFormat fmt;
    fmt.setFontUnderline(f_underline->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void Escriba::textItalic() {
    QTextCharFormat fmt;
    fmt.setFontItalic(f_italic->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void Escriba::textStrikeout() {
    QTextCharFormat fmt;
    fmt.setFontStrikeOut(f_strikeout->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void Escriba::textSize(const QString &p) {
    qreal pointSize = p.toDouble();
    if (p.toFloat() > 0) {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        mergeFormatOnWordOrSelection(fmt);
    }
}

void Escriba::textLink(bool checked) {
    bool unlink = false;
    QTextCharFormat fmt;
    if (checked) {
        QString url = f_richTextEdit->currentCharFormat().anchorHref();
        bool ok;
        QString newUrl = QInputDialog::getText(this, tr("Create a link"),
                                               tr("Link URL:"), QLineEdit::Normal,
                                               url,
                                               &ok);
        if (ok) {
            fmt.setAnchor(true);
            fmt.setAnchorHref(newUrl);
            fmt.setForeground(QApplication::palette().color(QPalette::Link));
            fmt.setFontUnderline(true);
        } else {
            unlink = true;
        }
    } else {
        unlink = true;
    }
    if (unlink) {
        fmt.setAnchor(false);
        fmt.setForeground(QApplication::palette().color(QPalette::Text));
        fmt.setFontUnderline(false);
    }
    mergeFormatOnWordOrSelection(fmt);
}

void Escriba::textStyle(int index) {
    QTextCursor cursor = f_richTextEdit->textCursor();
    QTextBlockFormat blockFormat = cursor.blockFormat();
    cursor.beginEditBlock();

    // standard
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::BlockUnderCursor);
    }
    QTextCharFormat fmt;
    cursor.setCharFormat(fmt);

    f_richTextEdit->setCurrentCharFormat(fmt);

    if ( index == ParagraphHeading1 ||
         index == ParagraphHeading2 ||
         index == ParagraphHeading3 ||
         index == ParagraphHeading4 )
    {
        int heading_property = -255; // Arbitrary number that should never come up
        switch (index) {
        case ParagraphHeading1:
            heading_property = 3;
            break;
        case ParagraphHeading2:
            heading_property = 2;
            break;
        case ParagraphHeading3:
            heading_property = 1;
            break;
        case ParagraphHeading4:
            heading_property = 0;
            break;
            //        // ParagraphHeading5 does not exit (yet)
            //        case ParagraphHeading5:
            //            heading_property = -1;
            //            break;
        case ParagraphMonospace:
        {
            fmt = cursor.charFormat();
            fmt.setFontFamily("Monospace");
            fmt.setFontStyleHint(QFont::Monospace);
            fmt.setFontFixedPitch(true);
            break;
        }
        }
        if (heading_property != -255) {
            fmt.setProperty(QTextFormat::FontSizeAdjustment, int(heading_property));
            QString c = QString("fmt.setProperty(QTextFormat::FontSizeAdjustment, int(%1));").arg(heading_property);
        }
    }

    // Wrap things up
    cursor.setCharFormat(fmt);
    m_curlineformat = fmt;
    cursor.setBlockCharFormat(fmt);
    f_richTextEdit->setCurrentCharFormat(fmt);
    cursor.endEditBlock();
}

void Escriba::textFgColor() {
    QColor col = QColorDialog::getColor(f_richTextEdit->textColor(), this);
    QTextCursor cursor = f_richTextEdit->textCursor();
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }
    QTextCharFormat fmt = cursor.charFormat();
    if (col.isValid()) {
        fmt.setForeground(col);
    } else {
        fmt.clearForeground();
    }
    cursor.setCharFormat(fmt);
    f_richTextEdit->setCurrentCharFormat(fmt);
    fgColorChanged(col);
}

void Escriba::textBgColor() {
    QColor col = QColorDialog::getColor(f_richTextEdit->textBackgroundColor(), this);
    QTextCursor cursor = f_richTextEdit->textCursor();
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }
    QTextCharFormat fmt = cursor.charFormat();
    if (col.isValid()) {
        fmt.setBackground(col);
    } else {
        fmt.clearBackground();
    }
    cursor.setCharFormat(fmt);
    f_richTextEdit->setCurrentCharFormat(fmt);
    bgColorChanged(col);
}

void Escriba::listBullet(bool checked) {
    if (checked) {
        f_list_ordered->setChecked(false);
    }
    list(checked, QTextListFormat::ListDisc);
}

void Escriba::listOrdered(bool checked) {
    if (checked) {
        f_list_bullet->setChecked(false);
    }
    list(checked, QTextListFormat::ListDecimal);
}

void Escriba::list(bool checked, QTextListFormat::Style style) {
    QTextCursor cursor = f_richTextEdit->textCursor();
    cursor.beginEditBlock();
    if (!checked) {
        QTextBlockFormat obfmt = cursor.blockFormat();
        QTextBlockFormat bfmt;
        bfmt.setIndent(obfmt.indent());
        cursor.setBlockFormat(bfmt);
    } else {
        QTextListFormat listFmt;
        if (cursor.currentList()) {
            listFmt = cursor.currentList()->format();
        }
        listFmt.setStyle(style);
        cursor.createList(listFmt);
    }
    cursor.endEditBlock();
}

void Escriba::mergeFormatOnWordOrSelection(const QTextCharFormat &format) {
    QTextCursor cursor = f_richTextEdit->textCursor();
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }
    cursor.mergeCharFormat(format);
    f_richTextEdit->mergeCurrentCharFormat(format);
    f_richTextEdit->setFocus(Qt::TabFocusReason);
}

void Escriba::switchedEditorType(int index)
{
    if (index) { // index == 1. This means user clicked Markdown tab
        // Ok, we must convert HTML (rich-text) to Markdown
        QString html = QBasicHtmlExporter(f_richTextEdit->document()).toHtml();
        m_mdpanda->loadHtmlString( html );
        f_plainTextEdit->document()->setPlainText( m_mdpanda->markdown() );
    } else { // index == 0. This means user clicked Fancy tab
        // Ok, we must convert Markdown (rich-text) to HTML
        m_mdpanda->loadMarkdownString( f_plainTextEdit->toPlainText() );
        f_richTextEdit->setText( m_mdpanda->html() );
    }
}

void Escriba::slotCursorPositionChanged() {
    QTextList *l = f_richTextEdit->textCursor().currentList();
    if (m_lastBlockList && (l == m_lastBlockList || (l != nullptr && m_lastBlockList != nullptr
                                                     && l->format().style() == m_lastBlockList->format().style()))) {
        return;
    }
    m_lastBlockList = l;
    if (l) {
        QTextListFormat lfmt = l->format();
        if (lfmt.style() == QTextListFormat::ListDisc) {
            f_list_bullet->setChecked(true);
            f_list_ordered->setChecked(false);
        } else if (lfmt.style() == QTextListFormat::ListDecimal) {
            f_list_bullet->setChecked(false);
            f_list_ordered->setChecked(true);
        } else {
            f_list_bullet->setChecked(false);
            f_list_ordered->setChecked(false);
        }
    } else {
        f_list_bullet->setChecked(false);
        f_list_ordered->setChecked(false);
    }
}

void Escriba::fontChanged(const QFont &f) {
    //f_fontsize->setCurrentIndex(f_fontsize->findText(QString::number(f.pointSize())));
    f_bold->setChecked(f.bold());
    f_italic->setChecked(f.italic());
    f_underline->setChecked(f.underline());
    f_strikeout->setChecked(f.strikeOut());

    if ( m_curlineformat.hasProperty( QTextFormat::FontSizeAdjustment ) )
    {
        switch ( m_curlineformat.intProperty(QTextFormat::FontSizeAdjustment) ) {
        case 3:
            f_paragraph->setCurrentIndex(ParagraphHeading1);
            break;
        case 2:
            f_paragraph->setCurrentIndex(ParagraphHeading2);
            break;
        case 1:
            f_paragraph->setCurrentIndex(ParagraphHeading3);
            break;
        case 0:
            f_paragraph->setCurrentIndex(ParagraphHeading4);
            break;
        //        case -1:
        //            f_paragraph->setCurrentIndex(ParagraphHeading5);
        //            break;
        default:
            if (f.fixedPitch() && f.family() == "Monospace") {
                f_paragraph->setCurrentIndex(ParagraphMonospace);
            } else {
                f_paragraph->setCurrentIndex(ParagraphStandard);
            }
            break;
        }
    }

//    if (f.pointSize() == m_fontsize_h1) {
//        f_paragraph->setCurrentIndex(ParagraphHeading1);
//    } else if (f.pointSize() == m_fontsize_h2) {
//        f_paragraph->setCurrentIndex(ParagraphHeading2);
//    } else if (f.pointSize() == m_fontsize_h3) {
//        f_paragraph->setCurrentIndex(ParagraphHeading3);
//    } else if (f.pointSize() == m_fontsize_h4) {
//        f_paragraph->setCurrentIndex(ParagraphHeading4);
//    } else {
//        if (f.fixedPitch() && f.family() == "Monospace") {
//            f_paragraph->setCurrentIndex(ParagraphMonospace);
//        } else {
//            f_paragraph->setCurrentIndex(ParagraphStandard);
//        }
//    }
    if (f_richTextEdit->textCursor().currentList()) {
        QTextListFormat lfmt = f_richTextEdit->textCursor().currentList()->format();
        if (lfmt.style() == QTextListFormat::ListDisc) {
            f_list_bullet->setChecked(true);
            f_list_ordered->setChecked(false);
        } else if (lfmt.style() == QTextListFormat::ListDecimal) {
            f_list_bullet->setChecked(false);
            f_list_ordered->setChecked(true);
        } else {
            f_list_bullet->setChecked(false);
            f_list_ordered->setChecked(false);
        }
    } else {
        f_list_bullet->setChecked(false);
        f_list_ordered->setChecked(false);
    }
}

void Escriba::fgColorChanged(const QColor &c) {
    QPixmap pix(16, 16);
    if (c.isValid()) {
        pix.fill(c);
    } else {
        pix.fill(QApplication::palette().foreground().color());
    }
    //    f_fgcolor->setIcon(pix);
}

void Escriba::bgColorChanged(const QColor &c) {
    QPixmap pix(16, 16);
    if (c.isValid()) {
        pix.fill(c);
    } else {
        pix.fill(QApplication::palette().background().color());
    }
    //    f_bgcolor->setIcon(pix);
}

void Escriba::slotCurrentCharFormatChanged(const QTextCharFormat &format) {
    fontChanged(format.font());
    bgColorChanged((format.background().isOpaque()) ? format.background().color() : QColor());
    fgColorChanged((format.foreground().isOpaque()) ? format.foreground().color() : QColor());
    f_link->setChecked(format.isAnchor());
}

void Escriba::slotClipboardDataChanged() {
#ifndef QT_NO_CLIPBOARD
    //    if (const QMimeData *md = QApplication::clipboard()->mimeData())
    //        f_paste->setEnabled(md->hasText());
#endif
}

QString Escriba::toHtml() const {
    QString s = f_richTextEdit->toHtml();
    // convert emails to links
    s = s.replace(QRegExp("(<[^a][^>]+>(?:<span[^>]+>)?|\\s)([a-zA-Z\\d]+@[a-zA-Z\\d]+\\.[a-zA-Z]+)"), "\\1<a href=\"mailto:\\2\">\\2</a>");
    // convert links
    s = s.replace(QRegExp("(<[^a][^>]+>(?:<span[^>]+>)?|\\s)((?:https?|ftp|file)://[^\\s'\"<>]+)"), "\\1<a href=\"\\2\">\\2</a>");
    // see also: Utils::linkify()
    return s;
}

void Escriba::increaseIndentation() {
    indent(+1);
}

void Escriba::decreaseIndentation() {
    indent(-1);
}

void Escriba::indent(int delta) {
    QTextCursor cursor = f_richTextEdit->textCursor();
    cursor.beginEditBlock();
    QTextBlockFormat bfmt = cursor.blockFormat();
    int ind = bfmt.indent();
    if (ind + delta >= 0) {
        bfmt.setIndent(ind + delta);
    }
    cursor.setBlockFormat(bfmt);
    cursor.endEditBlock();
}

void Escriba::setText(const QString& text) {
    if (text.isEmpty()) {
        setPlainText(text);
        return;
    }
    if (text[0] == '<') {
        setHtml(text);
    } else {
        setPlainText(text);
    }
}

void Escriba::insertImage() {
    QSettings s;
    QString attdir = s.value("general/filedialog-path").toString();
    QString file = QFileDialog::getOpenFileName(this,
                                                tr("Select an image"),
                                                attdir,
                                                tr("JPEG (*.jpg);; GIF (*.gif);; PNG (*.png);; BMP (*.bmp);; All (*)"));
    QImage image = QImageReader(file).read();

    f_richTextEdit->dropImage(image, QFileInfo(file).suffix().toUpper().toLocal8Bit().data() );

}

Escriba::~Escriba()
{
    delete m_mdpanda;
}

