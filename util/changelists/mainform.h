#ifndef MAINFORM_H
#define MAINFORM_H

#include "mainformbase.h"
#include <qprocess.h>
#include <qobject.h>

class MainForm : public MainFormBase
{
    Q_OBJECT

public:
    MainForm();
    ~MainForm();

private slots:
    void selectPath();
    void go();
    void currentChanged( QListViewItem * );

    void readyReadStdout();
    void readyReadStderr();
    void processExited();

private:
    void startChanges( QString label );

    QProcess process;
    QValueList<int> *changeListFrom;
    QValueList<int> *changeListTo;
};

#endif // MAINFORM_H
