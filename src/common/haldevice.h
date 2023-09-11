
#ifndef HALDEVICE_H
#define HALDEVICE_H

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <QObject>
#include <QDBusConnection>
#include <QDBusInterface>
//#include <QDBusAbstractInterface>
#include <QDBusObjectPath>
#include <QDBusReply>
#include <QDBusVariant>
#include <QTimer>
#include <QDBusMetaType>

#include <QFile>

#include <QMetaType>

//#include "filebrowser.h"

//#ifdef DEBUG_LOG
//    #define LOG(...) printf(__VA_ARGS__)
//#else
//    #define LOG( format, ... )  "\n"
//#endif

#define DBUS_HAL_SERVICE					"org.freedesktop.Hal" 
#define DBUS_HAL_PATH						"/org/freedesktop/Hal/Manager" 
#define DBUS_HAL_INTERFACE					"org.freedesktop.Hal.Manager" 

#define DBUS_HAL_DEVICE_INTERFACE			"org.freedesktop.Hal.Device" 

struct ChangeStruct
{
	QString propertyName;
	bool added;
	bool removed;
};
Q_DECLARE_METATYPE(ChangeStruct)
Q_DECLARE_METATYPE(QList<ChangeStruct>)

class haldevice : public QObject
{
    Q_OBJECT
signals:
	void mountedStatusChanged(bool);
	void readyBrowserUsb(QString path);
public:
    haldevice(QDBusInterface *interface,const QString &udi);
   ~haldevice();

public slots:
	void propertyModified(int numChanges,const QList<ChangeStruct> &changes);
	void proMountedStatusChanged(bool);
	void proReadyBrowserUsb(QString path);
public:
	const QString& udi() const;
	const QString& parentUdi() const;
	const QString& blockName() const;
	const QString& category() const;
	const QString& bus() const;
	int majorNumber() const;
	bool isMounted() const;
	bool isMountedByDBus() const;
	const QString& currentMountPointByDBus();

	void setParentUdi(QString);
	void setBlockName(QString);
	void setCurrentMountPoint(QString);
	void setCategory(QString);
	void setBus(QString);
	void setIsMounted(bool);
	void setMajorNumber(int);
private:
	QString deviceUdi;
	QString deviceParentUdi;
	QString deviceBlockName;
	QString deviceCurrentMountPoint;
	QString deviceCategory;
	QString deviceBus;
	bool deviceIsMounted;
	int major;
    QDBusInterface *informationCenter;
};

#endif
