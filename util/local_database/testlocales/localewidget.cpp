#include <QtGui/QTableView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QItemDelegate>
#include <QtGui/QItemEditorFactory>
#include <QtGui/QDoubleSpinBox>

#include "localewidget.h"
#include "localemodel.h"

class DoubleEditorCreator : public QItemEditorCreatorBase
{
public:
    QWidget *createWidget(QWidget *parent) const {
        QDoubleSpinBox *w = new QDoubleSpinBox(parent);
        w->setDecimals(4);
        w->setRange(-10000.0, 10000.0);
        return w;
    }
    virtual QByteArray valuePropertyName() const {
        return QByteArray("value");
    }
};

class EditorFactory : public QItemEditorFactory
{
public:
    EditorFactory() {
        static DoubleEditorCreator double_editor_creator;
        registerEditor(QVariant::Double, &double_editor_creator);
    }
};

LocaleWidget::LocaleWidget(QWidget *parent)
    : QWidget(parent)
{
    m_model = new LocaleModel(this);
    m_view = new QTableView(this);

    QItemDelegate *delegate = qobject_cast<QItemDelegate*>(m_view->itemDelegate());
    Q_ASSERT(delegate != 0);
    static EditorFactory editor_factory;
    delegate->setItemEditorFactory(&editor_factory);

    m_view->setModel(m_model);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(m_view);
}
