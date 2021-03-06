/*
** Copyright (C) 2018 Doug Beney (OpenAppLibrary.org - FloeMedia, LLC)
** Contact: https://openapplibrary.org/contact
**
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

#ifndef ESCRIBA_H
#define ESCRIBA_H

#include <QWidget>
#include <QPointer>
#include "ui_escriba.h"
#include "markdownsyntax.h"
#include <escribahelper.h>

class MarkdownPandaQt;

/**
 * @Brief A simple rich-text editor
 */
class Escriba : public QWidget, protected Ui::Escriba {
	Q_OBJECT
public:
	Escriba(QWidget *parent = nullptr);
    ~Escriba();

    QWidget *addonsArea();

	QString toMarkdown() const;
	void    setMarkdown(QString markdown);
	QTextDocument *document() { return f_richTextEdit->document(); }
	QTextCursor    textCursor() const { return f_richTextEdit->textCursor(); }
	void           setTextCursor(const QTextCursor& cursor) { f_richTextEdit->setTextCursor(cursor); }

public slots:
    void focusEditor();

protected slots:
	void textRemoveFormat();
	void textRemoveAllFormat();
	void textBold();
	void textUnderline();
	void textStrikeout();
	void textItalic();
	void textSize(const QString &p);
	void textLink(bool checked);
	void textStyle(int index);
	void textFgColor();
	void textBgColor();
	void listBullet(bool checked);
	void listOrdered(bool checked);
	void slotCurrentCharFormatChanged(const QTextCharFormat &format);
	void slotCursorPositionChanged();
	void slotClipboardDataChanged();
	void increaseIndentation();
	void decreaseIndentation();
	void insertImage();
	void textSource();
	void switchedEditorType(int index); // User has clicked the "Fancy" or "Markdown" tab

    void slot_documentChanged();

signals:
    void documentChanged(QString markdown);

protected:
	void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
	void fontChanged(const QFont &f);
	void fgColorChanged(const QColor &c);
	void bgColorChanged(const QColor &c);
	void list(bool checked, QTextListFormat::Style style);
	void indent(int delta);
	void focusInEvent(QFocusEvent *event);

	QStringList m_paragraphItems;
	int m_fontsize_h1;
	int m_fontsize_h2;
	int m_fontsize_h3;
	int m_fontsize_h4;

	enum ParagraphItems { ParagraphStandard,
												ParagraphHeading1,
												ParagraphHeading2,
												ParagraphHeading3,
												ParagraphHeading4,
												ParagraphMonospace };
	enum EditorTypes {FancyEditor, MarkdownEditor};

	QPointer<QTextList> m_lastBlockList;

private:
	MarkdownSyntax *m_markdownSyntax;
	MarkdownPandaQt *m_mdpanda;
	EditorTypes m_active_editor=FancyEditor;
};

#endif
