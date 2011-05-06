#ifndef FOOBARCLASS_H
#define FOOBARCLASS_H

#include <QObject>
#include <MwtsCommon>

//extern MwtsApp* g_pApp;

class FooBarClass : public QObject
{
    Q_OBJECT
public:
    explicit FooBarClass(QObject *parent = 0);

	void Initialize();

signals:

public slots:
	void Start();

};

#endif // FOOBARCLASS_H
