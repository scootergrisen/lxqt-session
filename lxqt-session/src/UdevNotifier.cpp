/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * http://lxqt.org
 *
 * Copyright: 2015 LXQt team
 * Authors:
 *  Palo Kisa <palo.kisa@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "UdevNotifier.h"
#include "log.h"
#include <libudev.h>
#include <QSocketNotifier>


class UdevNotifier::Impl
{
public:
    struct udev * udev;
    struct udev_monitor * monitor;
    QScopedPointer<QSocketNotifier> notifier;
};


UdevNotifier::UdevNotifier(QString const & subsystem, QObject * parent/* = nullptr*/)
    : QObject(parent)
    , d(new Impl)
{
    d->udev = udev_new();
    d->monitor = udev_monitor_new_from_netlink(d->udev, "udev");
    if (nullptr == d->monitor)
    {
        qCWarning(SESSION) << QStringLiteral("UdevNotifier: unable to initialize udev_monitor, monitoring will be disabled");
        return;
    }

    int ret = udev_monitor_filter_add_match_subsystem_devtype(d->monitor, subsystem.toUtf8().constData(), nullptr);
    if (0 != ret)
        qCWarning(SESSION) << QStringLiteral("UdevNotifier: unable to add match subsystem, monitor will receive all devices");

    ret = udev_monitor_enable_receiving(d->monitor);
    if (0 != ret)
    {
        qCWarning(SESSION) << QStringLiteral("UdevNotifier: unable to enable receiving(%1), monitoring will be disabled").arg(ret);
        return;
    }

    d->notifier.reset(new QSocketNotifier(udev_monitor_get_fd(d->monitor), QSocketNotifier::Read));
    connect(d->notifier.data(), &QSocketNotifier::activated, this, &UdevNotifier::eventReady);
    d->notifier->setEnabled(true);
}

UdevNotifier::~UdevNotifier()
{
    if (d->monitor)
        udev_monitor_unref(d->monitor);
    udev_unref(d->udev);
}

void UdevNotifier::eventReady(int socket)
{
    struct udev_device * dev;
    while (nullptr != (dev = udev_monitor_receive_device(d->monitor)))
    {
        QString const action = udev_device_get_action(dev);
        QString const device = udev_device_get_devpath(dev);

        if (QStringLiteral("add") == action)
            emit deviceAdded(std::move(device));
        else if (QStringLiteral("remove") == action)
            emit deviceRemoved(std::move(device));
        else if (QStringLiteral("change") == action)
            emit deviceChanged(std::move(device));
        else if (QStringLiteral("online") == action)
            emit deviceOnline(std::move(device));
        else if (QStringLiteral("offline") == action)
            emit deviceOffline(std::move(device));

        udev_device_unref(dev);
    }
}
