#ifndef FOOBARCLASS_H
#define FOOBARCLASS_H

#include <QObject>
#include <MwtsCommon>
#include <QDir>

//extern MwtsApp* g_pApp;

class FooBarClass : public QObject
{
    Q_OBJECT
public:
    explicit FooBarClass(QObject *parent = 0);

	void Initialize();

	virtual QString CaseName();
	void SetCaseName(QString sName);

	virtual void OnInitialize();
	virtual void OnUninitialize();

signals:

public slots:
	void Start();

protected:
	int m_nTestTimeout;
	int m_nFailTimeout;
	bool m_bFailTimeout;
	QTimer* m_pIdleTimer;
	QTimer* m_pFailTimeoutTimer;
	QTimer* m_pTestTimeoutTimer;
	QString m_sCaseName;

};

#endif // FOOBARCLASS_H
