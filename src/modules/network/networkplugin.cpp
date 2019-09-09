#include "networkplugin.h"
#include "networklistmodel.h"

#include "../../frame/item/pluginsitem.h"
#include "../../frame/mainpanel.h"

#include <DHiDPIHelper>

using namespace dtb;
using namespace dtb::network;
using namespace dde;
using namespace dde::network;

DWIDGET_USE_NAMESPACE

NetworkPlugin::NetworkPlugin()
{
    m_networkWidget = new FontLabel;
    m_networkWidget->setIcon(DHiDPIHelper::loadNxPixmap(QString(":/wireless/resources/wireless/wireless-disconnect-symbolic.svg")));
    m_networkWidget->setContentsMargins(5, 0, 5, 0);

    m_delayRefreshTimer = new QTimer(this);
    m_delayRefreshActiveConnection = new QTimer(this);
    m_networkModel = new NetworkModel;
    m_networkWorker = new NetworkWorker(m_networkModel);
    m_listModel = new NetworkListModel;
    m_controlPanel = new NetworkControlPanel(this, m_networkWorker, m_listModel);

    m_networkModel->moveToThread(qApp->thread());
    m_networkWorker->moveToThread(qApp->thread());
}

const QString NetworkPlugin::pluginName() const
{
    return "Network";
}

void NetworkPlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;

    m_delayRefreshTimer->setSingleShot(true);
    m_delayRefreshTimer->setInterval(2000);
    m_delayRefreshActiveConnection->setSingleShot(false);
    m_delayRefreshActiveConnection->setInterval(1000);

    connect(m_delayRefreshTimer, &QTimer::timeout, this, &NetworkPlugin::refreshWiredItemVisible);
    connect(m_delayRefreshActiveConnection, &QTimer::timeout, this, &NetworkPlugin::refreshActiveConnection);
    connect(m_networkModel, &NetworkModel::deviceListChanged, this, &NetworkPlugin::onDeviceListChanged);
    connect(m_networkModel, &NetworkModel::connectivityChanged, this, &NetworkPlugin::onConnectivityChanged);
    connect(m_networkModel, &NetworkModel::activeConnectionsChanged, this, &NetworkPlugin::onActiveConnectionsChanged);

    m_networkWorker->active();
    m_proxyInter->addItem(this, "network");

    PluginsItem* item = ((MainPanel*)m_proxyInter)->getItem(this, "network");
    connect(m_controlPanel, &NetworkControlPanel::sizeChanged, item, &PluginsItem::fitToContent);

    onDeviceListChanged(m_networkModel->devices());
}

QWidget *NetworkPlugin::itemWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    return m_networkWidget;
}

QWidget *NetworkPlugin::itemContextMenu(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    m_networkWorker->requestWirelessScan();

    return m_controlPanel;
}

void NetworkPlugin::setDefaultColor(PluginProxyInterface::DefaultColor color)
{
    Q_UNUSED(color);
}

void NetworkPlugin::refreshWiredItemVisible()
{

}

void NetworkPlugin::onDeviceListChanged(const QList<NetworkDevice *> devices)
{
    QList<QString> mPaths = m_itemsMap.keys();
    QList<QString> newPaths;

    for (auto device : devices) {
        const QString &path = device->path();
        newPaths << path;
        // new device
        if (!mPaths.contains(path)) {
            mPaths << path;
            m_itemsMap.insert(path, device);
            connect(device, &NetworkDevice::enableChanged, m_delayRefreshTimer, static_cast<void (QTimer:: *)()>(&QTimer::start));
        }
    }

    for (auto mPath : mPaths) {
        // removed device
        if (!newPaths.contains(mPath)) {
            m_itemsMap.take(mPath)->deleteLater();
            break;
        }
    }

    m_listModel->setDeviceList(m_itemsMap, this);

    m_delayRefreshTimer->start();
    m_delayRefreshActiveConnection->start();
}


void NetworkPlugin::onConnectivityChanged(Connectivity connectivity)
{
    if(connectivity == Connectivity::NoConnectivity || connectivity == Connectivity::UnknownConnectivity){

        m_networkWidget->setIcon(DHiDPIHelper::loadNxPixmap(QString(":/wireless/resources/wireless/wireless-disconnect-symbolic.svg")));
    } else if(connectivity == Connectivity::Limited || connectivity == Connectivity::Full){

        m_networkWidget->setIcon(DHiDPIHelper::loadNxPixmap(QString(":/wireless/resources/wireless/wireless-0-symbolic.svg")));
        refreshActiveConnection();
    }
}


void NetworkPlugin::refreshActiveConnection()
{
    QString ssid = "";
    for(auto conn : m_networkModel->activeConns()){
        // 0:Unknow, 1:Activating, 2:Activated, 3:Deactivating, 4:Deactivated
        int state = conn.value("State").toInt();
        if(state == 2) {
            ssid = conn.value("Id").toString();
            break;
        }
    }

    if(ssid == "") return;

    try
    {
        WirelessDevice *device = (WirelessDevice*)m_listModel->getCurrentNetworkDevice();
        if(!device) return;

        for (auto ap : device->apList()) {
            QJsonObject apObj = ap.toObject();
            if(apObj.value("Ssid") == ssid){
                onActiveAPInfoChanged(apObj);

                QModelIndex index = m_listModel->modelIndexBySsid(ssid);
                device->setActiveApBySsid(ssid);
                emit m_listModel->dataChanged(index, index);

                if(m_delayRefreshActiveConnection != nullptr){
                    m_delayRefreshActiveConnection->stop();
                    m_delayRefreshActiveConnection->deleteLater();
                    m_delayRefreshActiveConnection = nullptr;
                }

                break;
            }
        }
    }
    catch(const std::exception& e)
    {
        qErrnoWarning("Current device is not a wireless device.");
    }
}


void NetworkPlugin::onActiveAPInfoChanged(const QJsonObject &info)
{
    if(NetworkModel::connectivity() == Connectivity::NoConnectivity || NetworkModel::connectivity() == Connectivity::UnknownConnectivity){

        return;
    }

    int strength = info.value("Strength").toInt();
    int value = NetworkListModel::normalizeStrength(strength);

    QString iconPath = QString(":/wireless/resources/wireless/wireless-%1-symbolic.svg").arg(value);
    m_networkWidget->setIcon(DHiDPIHelper::loadNxPixmap(iconPath));
}


void NetworkPlugin::onActiveConnectionsChanged(const QList<QJsonObject> &activeConns)
{
    if(connectingSsid == "")
        return;

    for(auto conn : activeConns){
        QString connSsid = conn.value("Id").toString();
        if(connSsid != connectingSsid)
            continue;

        // 0:Unknow, 1:Activating, 2:Activated, 3:Deactivating, 4:Deactivated
        int state = conn.value("State").toInt();
        if(state == 2) {
            stopConnectingTimer();
            connectingSsid = "";
        }
        break;
    }
}


void NetworkPlugin::startConnectingTimer(QString ssid)
{
    connectingSsid = ssid;
    if(m_connectingTimer == nullptr){

        m_connectingTimer = new QTimer(this);
        m_connectingTimer->setSingleShot(false);
        m_connectingTimer->setInterval(500);
        
        connect(m_connectingTimer, &QTimer::timeout, this, &NetworkPlugin::timeoutConnectingTimer);

    }
    m_connectingTimer->start();
}


void NetworkPlugin::timeoutConnectingTimer()
{
  
    QString iconPath = QString(":/wireless/resources/wireless/wireless-%1-symbolic.svg").arg(connectingIcon);
    m_networkWidget->setIcon(DHiDPIHelper::loadNxPixmap(iconPath));
    
    connectingIcon = (connectingIcon + 2) % 10;
    if(connectingIcon == 0) 
        connectingIcon = 2;
}
    

void NetworkPlugin::stopConnectingTimer()
{

    if(m_connectingTimer != nullptr){
        
        m_connectingTimer->stop();
        m_connectingTimer->deleteLater();
        m_connectingTimer = nullptr;
        
        connectingIcon = 2;

    }
}
