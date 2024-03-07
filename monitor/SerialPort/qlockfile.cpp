/****************************************************************************
**
** Copyright (C) 2013 David Faure <faure+bluesystems@kde.org>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "QtCore/qlockfile.h"
#include "private/qlockfile_p.h"

#include <QtCore/qthread.h>
#include <QtCore/qelapsedtimer.h>
#include <QtCore/qdatetime.h>

QT_BEGIN_NAMESPACE

class QLockFileThread : public QThread
{
public:
    static void msleep(unsigned long msecs) { QThread::msleep(msecs); }
};

QLockFile::QLockFile(const QString &fileName)
    : d_ptr(new QLockFilePrivate(fileName))
{
}


QLockFile::~QLockFile()
{
    unlock();
}


void QLockFile::setStaleLockTime(int staleLockTime)
{
    Q_D(QLockFile);
    d->staleLockTime = staleLockTime;
}


int QLockFile::staleLockTime() const
{
    Q_D(const QLockFile);
    return d->staleLockTime;
}


bool QLockFile::isLocked() const
{
    Q_D(const QLockFile);
    return d->isLocked;
}


bool QLockFile::lock()
{
    return tryLock(-1);
}


bool QLockFile::tryLock(int timeout)
{
    Q_D(QLockFile);
    QElapsedTimer timer;
    if (timeout > 0)
        timer.start();
    int sleepTime = 100;
    forever {
        d->lockError = d->tryLock_sys();
        switch (d->lockError) {
        case NoError:
            d->isLocked = true;
            return true;
        case PermissionError:
        case UnknownError:
            return false;
        case LockFailedError:
            if (!d->isLocked && d->isApparentlyStale()) {
                // Stale lock from another thread/process
                // Ensure two processes don't remove it at the same time
                QLockFile rmlock(d->fileName + QLatin1String(".rmlock"));
                if (rmlock.tryLock()) {
                    if (d->isApparentlyStale() && d->removeStaleLock())
                        continue;
                }
            }
            break;
        }
        if (timeout == 0 || (timeout > 0 && timer.hasExpired(timeout)))
            return false;
        QLockFileThread::msleep(sleepTime);
        if (sleepTime < 5 * 1000)
            sleepTime *= 2;
    }
    // not reached
    return false;
}


bool QLockFile::getLockInfo(qint64 *pid, QString *hostname, QString *appname) const
{
    Q_D(const QLockFile);
    return d->getLockInfo(pid, hostname, appname);
}

bool QLockFilePrivate::getLockInfo(qint64 *pid, QString *hostname, QString *appname) const
{
    QFile reader(fileName);
    if (!reader.open(QIODevice::ReadOnly))
        return false;

    QByteArray pidLine = reader.readLine();
    pidLine.chop(1);
    QByteArray appNameLine = reader.readLine();
    appNameLine.chop(1);
    QByteArray hostNameLine = reader.readLine();
    hostNameLine.chop(1);
    if (pidLine.isEmpty() || appNameLine.isEmpty())
        return false;

    qint64 thePid = pidLine.toLongLong();
    if (pid)
        *pid = thePid;
    if (appname)
        *appname = QString::fromUtf8(appNameLine);
    if (hostname)
        *hostname = QString::fromUtf8(hostNameLine);
    return thePid > 0;
}


bool QLockFile::removeStaleLockFile()
{
    Q_D(QLockFile);
    if (d->isLocked) {
        qWarning("removeStaleLockFile can only be called when not holding the lock");
        return false;
    }
    return d->removeStaleLock();
}


QLockFile::LockError QLockFile::error() const
{
    Q_D(const QLockFile);
    return d->lockError;
}

QT_END_NAMESPACE
