/*********************************************************************
Component Organizer
Copyright (C) M?rio Ribeiro (mario.ribas@gmail.com)

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

#include "package.h"

#include <QStringList>

const QStringList Package::m_defaultNames =  //TODO sort the list
    (QStringList()
     << "Günes Gözlükleri"
     << "Numarali Gözlükler"
     << "Reçeteli Gözlükler"

    );

QStringList Package::defaultNames()
{
    return m_defaultNames;
}

Package::Package(const QString &name, QObject *parent) :
    QObject(parent),
    m_name(name)
{
}
