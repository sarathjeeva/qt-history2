/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PROJECTPORTER_H
#define PROJECTPORTER_H

#include "fileporter.h"
#include "preprocessorcontrol.h"
#include <QString>
#include <QSet>

QT_BEGIN_NAMESPACE

class ProjectPorter : public QObject
{
Q_OBJECT
public:
    ProjectPorter(QString basePath, QStringList includeDirectories, QStringList qt3HeadersFilenames = QStringList());
    void enableCppParsing(bool enable);
    void enableMissingFilesWarnings(bool enable);
    void portProject(QString filePath);
    void portFile(QString filePath);
private slots:
    void error(QString type, QString text);
private:
    void portProject(QString inPath, QString proFileName);
    void portProFile(QString fileName, QMap<QString, QString> tagMap);
    void portFiles(QString basePath, QStringList fileNames);
    void enableAttributes(const IncludeFiles &includeFiles, const QString &fileName);

    QSet<QString> processedFilesSet;
    QString basePath;
    QStringList includeDirectories;
    PreprocessorCache preprocessorCache;
    Rpp::DefineMap *defaultDefinitions;
    FilePorter filePorter;
    QStringList qt3HeadersFilenames;
    bool analyze;
    bool warnings;
};

QT_END_NAMESPACE

#endif
