#ifndef FEATURE_INCLUDED
#define FEATURE_INCLUDED

#include <QString>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QList>

class Feature;

class FeaturePrivate
{
public:
    FeaturePrivate(const QString &k)
	: key(k), enabled(true), selectable(true) {};
    
    const QString key;
    QString section;
    QString title;
    QString description;
    QSet<Feature*> dependencies;
    QSet<Feature*> supports; // features who depends on this one
    QSet<Feature*> relations;
    bool enabled;
    bool selectable;
};

class Feature : public QObject
{
    Q_OBJECT    

public:
    static Feature* getInstance(const QString &key);
    static void clear();

public:
    QString key() const { return d->key; }

    void setTitle(const QString &title);
    QString title() const { return d->title; }

    void setSection(const QString &section);
    QString section() const { return d->section; }

    void setDescription(const QString &description);    
    QString description() const { return d->description; };

    void addRelation(const QString &key);
    void setRelations(const QStringList &keys);
    QList<Feature*> relations() const;    

    void addDependency(const QString &dependency);
    void setDependencies(const QStringList &dependencies);
    QList<Feature*> dependencies() const;

    QList<Feature*> supports() const;    
    QString getDocumentation() const;

    void setEnabled(bool on);
    bool enabled() const { return d->enabled; };

    bool selectable() const { return d->selectable; }
    
    QString toHtml() const;    

    ~Feature();    

signals:
    void changed();
    
private:
    Feature(const QString &key);
    void updateSelectable(const Feature *changed);

    static QMap<QString, Feature*> instances;
    FeaturePrivate *d;
};

#endif // FEATURE_INCLUDED
