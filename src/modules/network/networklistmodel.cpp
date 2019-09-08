#include "networklistmodel.h"

#include <QDebug>
#include <QSize>

using namespace dtb;
using namespace dtb::network;

NetworkListModel::NetworkListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_currentWirelessDevice(nullptr)
{

}

void NetworkListModel::setDeviceList(const QMap<QString, dde::network::NetworkDevice *> list, NetworkPlugin *networkPlugin)
{
    m_wiredList.clear();
    m_wirelessList.clear();

    for (auto it = list.constBegin(); it != list.constEnd(); it++) {
        if (it.value()->type() == dde::network::NetworkDevice::Wired) {
            m_wiredList[it.key()] = static_cast<dde::network::WiredDevice*>(it.value());
        }
        else {
            dde::network::WirelessDevice *device = static_cast<dde::network::WirelessDevice*>(it.value());
            m_wirelessList[it.key()] = device;

            // set default wireless device
            if (!m_currentWirelessDevice) m_currentWirelessDevice = device;

            connect(device, &dde::network::WirelessDevice::apAdded, this, &NetworkListModel::APAdded);
            connect(device, &dde::network::WirelessDevice::apRemoved, this, &NetworkListModel::APRemoved);
            connect(device, &dde::network::WirelessDevice::apInfoChanged, this, &NetworkListModel::APPropertiesChanged);
            connect(device, &dde::network::WirelessDevice::activeApInfoChanged, networkPlugin, &NetworkPlugin::onActiveAPInfoChanged);

            QTimer::singleShot(1000, this, [=]{
                networkPlugin->onActiveAPInfoChanged(device->activeApInfo());
            });

            for (auto ap : device->apList()) {
                emit device->apAdded(ap.toObject());
            }
        }
    }

    // refresh all info
}

void NetworkListModel::setHoverIndex(const QModelIndex &index)
{
    m_hoverIndex = index;

    emit dataChanged(index, index);
}

int NetworkListModel::rowCount(const QModelIndex &parent) const
{
    if (!m_currentWirelessDevice) {
        return 0;
    }

    return m_apMap[m_currentWirelessDevice].size();
}

int NetworkListModel::rowCount()
{
    if (!m_currentWirelessDevice) return 0;
    return m_apMap[m_currentWirelessDevice].size();
}

AccessPoint* NetworkListModel::getAP(const QModelIndex &index)
{
    if (!m_currentWirelessDevice) return NULL;

    const int row { index.row() };

    return &m_apMap[m_currentWirelessDevice][row];
}

NetworkDevice* NetworkListModel::getCurrentNetworkDevice()
{
    return m_currentWirelessDevice;
}


QVariant NetworkListModel::data(const QModelIndex &index, int role) const
{
    if (!m_currentWirelessDevice) return QVariant();

    const int row { index.row() };

    switch (role) {
    case NameRole:
        return m_apMap[m_currentWirelessDevice][row].ssid();
    case SizeRole:
        return QSize(350, 34);
    case HoverRole:
        return index == m_hoverIndex;
    case IconRole:
        return icon(index);
    case SecurityRole:
        return isSecurity(index);
    default:
        break;
    }

    return QVariant();
}

void NetworkListModel::onActivateConnectChanged()
{

}

void NetworkListModel::deviceStateChanged()
{

}

void NetworkListModel::APAdded(const QJsonObject &obj)
{
    const AccessPoint ap(obj);
    dde::network::WirelessDevice* dev = static_cast<dde::network::WirelessDevice*>(sender());
    if (m_apMap[dev].contains(ap)) return;

    m_apMap[dev] << std::move(ap);

    // refresh
    emit layoutChanged();
}

void NetworkListModel::APRemoved(const QJsonObject &obj)
{
    const AccessPoint ap(obj);

    dde::network::WirelessDevice* dev = static_cast<dde::network::WirelessDevice*>(sender());
    m_apMap[dev].removeOne(ap);

    // refresh
    emit layoutChanged();
}

void NetworkListModel::APPropertiesChanged(const QJsonObject &apInfo)
{
    const AccessPoint ap(apInfo);

    dde::network::WirelessDevice* dev = static_cast<dde::network::WirelessDevice*>(sender());

    for (int i = 0; i != m_apMap[dev].size(); ++i) {
        if (m_apMap[dev][i] == ap) {
            m_apMap[dev][i] = ap;
            break;
        }
    }

    // refresh
    const QModelIndex i = index(m_apMap[dev].indexOf(ap));

    emit dataChanged(i, i);
}

bool NetworkListModel::isSecurity(const QModelIndex &index) const
{
    return m_apMap[m_currentWirelessDevice][index.row()].secured();
}

const QString NetworkListModel::icon(const QModelIndex &index) const
{
    int strength = m_apMap[m_currentWirelessDevice][index.row()].strength();

    int value = normalizeStrength(strength);

    return QString(":/wireless/resources/wireless/wireless-%1.svg").arg(value);
}

int NetworkListModel::normalizeStrength(int strength)
{
    strength /= 10;
    if(strength > 9) strength = 9;

    int value = 0;
    for (int i = 0; i <= 9; i++) {
        if (strength <= i) {
            value = i;
            break;
        }
    }

    if ((value % 2) != 0) {
        value -= 1;
    }

    return value;
}
