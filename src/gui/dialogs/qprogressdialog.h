/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPROGRESSDIALOG_H
#define QPROGRESSDIALOG_H

#include "qdialog.h"
#include "qlabel.h"       // ### remove or keep for users' convenience?
#include "qprogressbar.h" // ### remove or keep for users' convenience?

#ifndef QT_NO_PROGRESSDIALOG

class QPushButton;
class QTimer;
class QProgressDialogData;

class Q_GUI_EXPORT QProgressDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(bool wasCanceled READ wasCanceled)
    Q_PROPERTY(int totalSteps READ totalSteps WRITE setTotalSteps)
    Q_PROPERTY(int progress READ progress WRITE setProgress)
    Q_PROPERTY(bool autoReset READ autoReset WRITE setAutoReset)
    Q_PROPERTY(bool autoClose READ autoClose WRITE setAutoClose)
    Q_PROPERTY(int minimumDuration READ minimumDuration WRITE setMinimumDuration)
    Q_PROPERTY(QString labelText READ labelText WRITE setLabelText)

public:
    QProgressDialog(QWidget* parent=0, const char* name=0, bool modal=false,
                     Qt::WFlags f=0);
    QProgressDialog(const QString& labelText, const QString &cancelButtonText,
                     int totalSteps, QWidget* parent=0, const char* name=0,
                     bool modal=false, Qt::WFlags f=0);
    ~QProgressDialog();

    void setLabel(QLabel *);
    void setCancelButton(QPushButton *);
    void setBar(QProgressBar *);

    bool wasCanceled() const;

    int totalSteps() const;
    int progress()   const;

    QSize sizeHint() const;

    QString labelText() const;

    void setAutoReset(bool b);
    bool autoReset() const;
    void setAutoClose(bool b);
    bool autoClose() const;

public slots:
    void cancel();
    void reset();
    void setTotalSteps(int totalSteps);
    void setProgress(int progress);
    void setProgress(int progress, int totalSteps);
    void setLabelText(const QString &);
    void setCancelButtonText(const QString &);

    void setMinimumDuration(int ms);
public:
    int minimumDuration() const;

signals:
    void canceled();

protected:
    void resizeEvent(QResizeEvent *);
    void closeEvent(QCloseEvent *);
    void changeEvent(QEvent *);
    void showEvent(QShowEvent *e);

protected slots:
    void forceShow();

private:
    void init(QWidget *creator, const QString& lbl, const QString &canc,
                         int totstps);
    void layout();
    QLabel *label()  const;
    QProgressBar *bar()    const;
    QProgressDialogData *d;
    QTimer *forceTimer;

private:
    Q_DISABLE_COPY(QProgressDialog)
};

#endif // QT_NO_PROGRESSDIALOG

#endif // QPROGRESSDIALOG_H
