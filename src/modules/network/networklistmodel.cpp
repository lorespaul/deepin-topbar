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
            connect(device, &dde::network::WirelessDevice::activeConnectionsChanged, networkPlugin, &NetworkPlugin::onActiveConnectionsChanged);

            for (auto ap : device->apList()) {
                emit device->apAdded(ap.toObject());
            }
        }
    }

    // refresh all info
}

void NetworkListModel::removeHoverIndex()
{
    m_hoverIndex = this->index(-1);

    emit dataChanged(m_hoverIndex, m_hoverIndex);
}

void NetworkListModel::setHoverIndex(const QModelIndex &index)
{
    m_hoverIndex = index;

    emit dataChanged(index, index);
}

int NetworkListModel::rowCount(const QModelIndex &parent) const
{
    if (!m_currentWirelessDevice) {
        return 1;
    }

    int apSize = m_apMap[m_currentWirelessDevice].size();
    return apSize == 0 ? 1 : apSize;
}


AccessPoint* NetworkListModel::getAP(const QModelIndex &index)
{
    if (!m_currentWirelessDevice) return NULL;

    const int row { index.row() };

    if(m_apMap[m_currentWirelessDevice].size() > row)
        return &m_apMap[m_currentWirelessDevice][row];
    return nullptr;
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
        if(m_apMap[m_currentWirelessDevice].size() == 0)
            return tr("No access point found...");
        return m_apMap[m_currentWirelessDevice][row].ssid();
    case SizeRole:
        return QSize(300, 34);
    case HoverRole:
        return index == m_hoverIndex;
    case ActiveRole:
        if(m_apMap[m_currentWirelessDevice].size() == 0)
            return false;
        return m_currentWirelessDevice->activeApSsid() == m_apMap[m_currentWirelessDevice][row].ssid();
    case IconRole:
        if(m_apMap[m_currentWirelessDevice].size() == 0)
            return QVariant();
        return icon(index);
    case SecurityRole:
        if(m_apMap[m_currentWirelessDevice].size() == 0)
            return false;
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

    if(m_cacheApMap != nullptr){
        m_cacheApMap->append(ap);
        return;
    }

    m_apMap[dev] << std::move(ap);

    // refresh
    emit layoutChanged();
}

void NetworkListModel::APRemoved(const QJsonObject &obj)
{
    const AccessPoint ap(obj);

    dde::network::WirelessDevice* dev = static_cast<dde::network::WirelessDevice*>(sender());

    if(m_cacheApMap != nullptr){
        m_cacheApMap->removeOne(ap);
        return;
    }

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

    if(m_cacheApMap != nullptr)
        return;

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


void NetworkListModel::toggleList(bool active)
{
    if(!m_currentWirelessDevice) return;

    if(active){

        if(m_cacheApMap != nullptr && m_cacheApMap->size() > 0){
            m_apMap[m_currentWirelessDevice].append(*m_cacheApMap);
            m_cacheApMap = nullptr;
        }

    } else {

        if(m_apMap[m_currentWirelessDevice].size() > 0)
            m_cacheApMap = new QList<AccessPoint>(m_apMap[m_currentWirelessDevice]);
        else 
            m_cacheApMap = new QList<AccessPoint>();
        m_apMap[m_currentWirelessDevice].clear();

    }
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


QModelIndex NetworkListModel::modelIndexBySsid(QString ssid)
{
    for(int i = 0; i < m_apMap[m_currentWirelessDevice].size(); i++){
        AccessPoint ap = m_apMap[m_currentWirelessDevice][i];
        if(ap.ssid() == ssid)
            return index(i);
    }
    return index(-1);
}
