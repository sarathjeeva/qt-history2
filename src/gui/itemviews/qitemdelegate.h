#ifndef QITEMDELEGATE_H
#define QITEMDELEGATE_H

#include <qabstractitemdelegate.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qvariant.h>

class Q_GUI_EXPORT QItemDelegate : public QAbstractItemDelegate
{
public:
    QItemDelegate(QAbstractItemModel *model, QObject *parent = 0);
    virtual ~QItemDelegate();

    // painting
    void paint(QPainter *painter, const QItemOptions &options, const QModelIndex &item) const;
    QSize sizeHint(const QFontMetrics &fontMetrics, const QItemOptions &options,
                   const QModelIndex &item) const;

    // editing
    QAbstractItemDelegate::EditType editType(const QModelIndex &item) const;
    QWidget *createEditor(QAbstractItemDelegate::StartEditAction action, QWidget *parent,
                          const QItemOptions &options, const QModelIndex &item);
    void setContentFromEditor(QWidget *editor, const QModelIndex &item) const;
    void updateEditorContents(QWidget *editor, const QModelIndex &item) const;
    void updateEditorGeometry(QWidget *editor, const QItemOptions &options,
                              const QModelIndex &item) const;
    void removeEditor(EndEditAction action, QWidget *editor, const QModelIndex &item);

protected:
    void drawText(QPainter *painter, const QItemOptions &options, const QRect &rect,
                  const QString &text) const;
    void drawPixmap(QPainter *painter, const QItemOptions &options, const QRect &rect,
                    const QPixmap &pixmap) const;
    void drawFocus(QPainter *painter, const QItemOptions &options, const QRect &rect) const;
    void doLayout(const QItemOptions &options, QRect *iconRect, QRect *textRect, bool hint) const;
    void doAlignment(const QRect &boundingRect, int alignment, QRect *rect) const;
    QPixmap decoration(const QItemOptions &options, const QVariant &variant) const;
};

#endif
