/*********************************************************************
Component Organizer
Copyright (C) M�rio Ribeiro (mario.ribas@gmail.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#include "datasheet.h"

#include <QStringList>

QString Datasheet::typeToString(Type type)
{
    switch(type)
    {
        case Datasheet::Normal:
            return QString(tr("Resmi Evrak"));
        case Datasheet::Summary:
            return QString(tr("Katalog"));
        case Datasheet::Errata:
            return QString(tr("Klavuz"));
        case Datasheet::UsersGuide:
            return QString(tr("�retici Notu"));
        default:
            return QString();
    }
}

Datasheet::Type Datasheet::typeFromString(QString str)
{
    if(str.compare(tr("Resmi Evrak")) == 0)
    {
        return Datasheet::Normal;
    }
    else if(str.compare(tr("Katalog")) == 0)
    {
        return Datasheet::Summary;
    }
    else if(str.compare(tr("Klavuz")) == 0)
    {
        return Datasheet::Errata;
    }
    else if(str.compare(tr("�retici Notu")) == 0)
    {
        return Datasheet::UsersGuide;
    }
    else
        return (Datasheet::Type) - 1;
}

QStringList Datasheet::typeNames()
{
    QStringList list;
    list.append(typeToString(Datasheet::Normal));
    list.append(typeToString(Datasheet::Summary));
    list.append(typeToString(Datasheet::Errata));
    list.append(typeToString(Datasheet::UsersGuide));
    return list;
}

Datasheet::Datasheet(const QString &path, QObject *parent) :
    QObject(parent),
    m_path(path)
{
}
