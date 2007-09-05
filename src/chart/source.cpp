/*
 * Copyright (c) 2006, Ken McDonell.  All Rights Reserved.
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

//
// Source Class
//
// manipulate one or more PMAPI contexts

#include <QtGui/QMessageBox>
#include "namespace.h"
#include "main.h"

#define DESPERATE 1

QList<PMC_Context *>contextList;
PMC_Context *currentContext;

Source::Source(PMC_Group *group)
{
    my.fetchGroup = group;
    my.context = NULL;
}

QString Source::makeSourceBaseName(const PMC_Context *cp)
{
    if (cp->source().type() == PM_CONTEXT_ARCHIVE)
	return QString(cp->source().source().ptr());
    return QString(cp->source().host().ptr());
}

QString Source::makeSourceAnnotatedName(const PMC_Context *cp)
{
    QString t;

    if (cp->source().type() == PM_CONTEXT_HOST) {
	t = QString(cp->source().host().ptr());
	// TODO - pmproxy host support needed here.
	t.append(" (no proxy)");
    }
    else {
	t = QString(cp->source().source().ptr());
	t.append(" (");
	t.append(cp->source().host().ptr());
	t.append(")");
    }
    return t;
}

QString Source::makeComboText(const PMC_Context *ctx)
{
    return makeSourceAnnotatedName(ctx);
}

int useSourceContext(QWidget *parent, QString &source)
{
    int sts;
    PMC_Group *group = activeTab->group();
    uint_t ctxcount = group->numContexts();
    const char *src = source.toAscii();
    char *end, *proxy, *host = strdup(src);

    // TODO: proxy support needed (we toss proxy hostname atm)
    end = host;
    strsep(&end, " ");
    proxy = strsep(&end, ")") + 1;
    *end = '\0';

    console->post("useSourceContext trying new source: %s; host=%s proxy=%s\n",
			src, host, proxy);

    if (strcmp(proxy, "no proxy") == 0)
	unsetenv("PMPROXY_HOST");
    else
	setenv("PMPROXY_HOST", proxy, 1);
    if ((sts = group->use(activeSources->type(), host)) < 0) {
	QString msg = QString();
	msg.sprintf("Failed to %s \"%s\".\n%s.\n\n",
		    (activeSources->type() == PM_CONTEXT_HOST) ?
		    "connect to pmcd on" : "open archive", host, pmErrStr(sts));
	QMessageBox::warning(parent, pmProgname, msg,
		QMessageBox::Ok | QMessageBox::Default | QMessageBox::Escape,
		QMessageBox::NoButton, QMessageBox::NoButton);
    }
    else if (group->numContexts() > ctxcount)
	activeSources->add(group->which());
    free(host);
    return sts;
}

int Source::useComboContext(QWidget *parent, QComboBox *combo)
{
    QString source = combo->currentText();
    int sts = useSourceContext(parent, source);
    if (sts < 0)
	combo->removeItem(0);
    return sts;
}

int Source::type()
{
    return currentContext ? currentContext->source().type() : -1;
}

QString Source::host()
{
    // TODO: nathans - these aint QString's, theyre char* ... hmm?
    return my.context ? my.context->source().host().ptr() : NULL;
}

const char *Source::source()
{
#if DESPERATE
    console->post("Source::source(): currentContext=%p", currentContext);
#endif

    if (!currentContext)
    	return NULL;

#if DESPERATE
    console->post("  currentContext hndl=%d source=%s", currentContext->hndl(),
		currentContext->source().source().ptr());
#endif

    return currentContext->source().source().ptr();
}

void Source::add(PMC_Context *context)
{
    bool send_bounds = (context->source().type() == PM_CONTEXT_ARCHIVE);

    contextList.append(context);
    currentContext = context;
    my.context = context;

#if DESPERATE
    dump(stderr);
#endif

    // For archives, send a message to kmtime to grow its time window.
    // This is already done if we're the first, so don't do it again;
    // we also don't have a kmtime connection yet if processing args.
    
    if (kmtime && send_bounds) {
	const PMC_Source *source = &context->source();
	PMC_String tz = source->timezone();
	PMC_String host = source->host();
	struct timeval logStartTime = source->start();
	struct timeval logEndTime = source->end();
	kmtime->addArchive(&logStartTime, &logEndTime, tz.ptr(),
				tz.length(), host.ptr(), host.length());
    }
}

void Source::dump(FILE *f)
{
    fprintf(f, "Source::dump: current=%p\n", currentContext);
    for (int i = 0; i < contextList.size(); i++) {
	PMC_Context *cp = contextList.at(i);
	fprintf(f, "context=%p\n", cp);
	fprintf(f, "  [%d] type=%d source=%s host=%s ctxt=%d\n", i,
		cp->source().type(), cp->source().source().ptr(),
		cp->source().host().ptr(), cp->hndl());
    }
}

void Source::setupCombo(QComboBox *combo)
{
    combo->clear();
    for (int i = 0; i < contextList.size(); i++) {
	PMC_Context *cp = contextList.at(i);
	QString source = makeComboText(cp);
	combo->insertItem(i, source);
	if (cp == currentContext)
	    combo->setCurrentIndex(i);
    }
}

void Source::setCurrentInCombo(QComboBox *combo)
{
    if (!currentContext)
	return;

    QString source = makeComboText(currentContext);
    for (int i = 0; i < combo->count(); i++) {
	if (combo->itemText(i) == source) {
	    combo->setCurrentIndex(i);
	    return;
	}
    }
}

// Called from a QComboBox widget, where name is setup by Source::setupCombo
//
void Source::setCurrentFromCombo(const QString name)
{
    for (int i = 0; i < contextList.size(); i++) {
	PMC_Context *cp = contextList.at(i);
	if (name == makeComboText(cp)) {
	    currentContext = cp;
	    return;
	}
    }
}

void Source::setupTree(QTreeWidget *tree)
{
    NameSpace *current = NULL;
    QList<QTreeWidgetItem*> items;

    tree->clear();
    for (int i = 0; i < contextList.size(); i++) {
	PMC_Context *cp = contextList.at(i);
	NameSpace *name = new NameSpace(tree, cp, activeTab->isArchiveSource());
	name->setOpen(true);
	name->setSelectable(false);
	tree->addTopLevelItem(name);
	if (cp == currentContext)
	    current = name;
	items.append(name);
    }
    tree->insertTopLevelItems(0, items);
    if (current)
    	tree->setCurrentItem(current);
}
