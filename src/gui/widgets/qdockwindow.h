#ifndef QDOCKWINDOW_H
#define QDOCKWINDOW_H

#include <qframe.h>

class QMainWindow;
class QDockWindowPrivate;

class Q_GUI_EXPORT QDockWindow : public QFrame
{
    Q_OBJECT

    Q_PROPERTY(bool closable READ isClosable WRITE setClosable)
    Q_PROPERTY(bool movable READ isMovable WRITE setMovable)
    Q_PROPERTY(bool floatable READ isFloatable WRITE setFloatable)
    Q_PROPERTY(DockWindowAreaFlags allowedAreas READ allowedAreas WRITE setAllowedAreas)
    Q_PROPERTY(Qt::DockWindowArea currentArea READ currentArea WRITE setCurrentArea)

    Q_DECLARE_PRIVATE(QDockWindow)

public:
    QDockWindow(QMainWindow *parent, WFlags flags = 0);
    QDockWindow(QMainWindow *parent, Qt::DockWindowArea area, WFlags flags = 0);
    ~QDockWindow();

    void setParent(QMainWindow *parent);
    QMainWindow *mainWindow() const;

    void setClosable(bool closable = true);
    bool isClosable() const;

    void setMovable(bool movable = true);
    bool isMovable() const;

    void setFloatable(bool floatable = true);
    bool isFloatable() const;

    void setFloated(bool floated = true, const QPoint &pos = QPoint());
    bool isFloated() const;

    void setAllowedAreas(DockWindowAreaFlags areas);
    DockWindowAreaFlags allowedAreas() const;

    inline bool isDockable(Qt::DockWindowArea area)
    { return (allowedAreas() & area) == area; }

    Qt::DockWindowArea currentArea() const;

    void setCurrentArea(Qt::DockWindowArea area); // always extends
    void setCurrentArea(Qt::DockWindowArea area, Qt::Orientation direction, bool extend = false);

    void setCurrentArea(QDockWindow *after, Qt::Orientation direction); // always splits

protected:
    void changeEvent(QEvent *event);
    void childEvent(QChildEvent *event);
    bool event(QEvent *event);
};

#endif // QDOCKWINDOW_H
