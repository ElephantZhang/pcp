/*
 * Copyright (c) 2007, Aconex.  All Rights Reserved.
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */
#ifndef KMQUERY_H
#define KMQUERY_H

#include <QtCore/QVariant>
#include <QtCore/QTimerEvent>

#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QTextEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

class KmQuery : public QDialog
{
    Q_OBJECT
public:
    KmQuery(bool inputflag, bool printflag, bool noframeflag,
	    bool nosliderflag, bool usesliderflag, bool exclusiveflag);
    void setStatus(int status) { my.status = status; }

    static void setTitle(char *string);
    static int setTimeout(char *string);
    static int setIcontype(char *string);

    static int messageCount();
    static void addMessage(char *string);

    static int buttonCount();
    static void addButton(char *string, bool iamdefault, int exitstatus);
    static void addButtons(char *stringlist);
    static void setDefaultButton(char *string);

public slots:
    void buttonClicked();

protected:
    void timerEvent(QTimerEvent *);

private:
    struct {
	int status;
    } my;
};

class QueryButton : public QPushButton
{
    Q_OBJECT
public:
    QueryButton(bool out, QWidget *p) : QPushButton(NULL, p)
	{
	    my.s = 0;
	    my.k = NULL;
	    my.l = NULL;
	    my.t = NULL;
	    if (out)
		connect(this, SIGNAL(clicked()), this, SLOT(print()));
	    else
		connect(this, SIGNAL(clicked()), this, SLOT(noprint()));
	}
    void setQuery(KmQuery *dialog) { my.k = dialog; }
    void setStatus(int status) { my.s = status; }
    void setEditor(QLineEdit *editor) { my.l = editor; }
    void setEditor(QTextEdit *editor) { my.t = editor; }

public slots:
    void print()
	{
	    noprint(); puts(my.l ? my.l->text().toAscii().data() : (my.t?
	    my.t->toPlainText().toAscii().data() : text().toAscii().data()));
	}
    void noprint() { my.k->setStatus(my.s); }

private:
    struct {
	int s;
	KmQuery *k;
	QLineEdit *l;
	QTextEdit *t;
    } my;
};

#endif // KMQUERY_H
