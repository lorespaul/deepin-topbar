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
    
    m_controlPanel = new NetworkControlPanel;
    m_delayRefreshTimer = new QTimer(this);
    m_networkModel = new NetworkModel;
    m_networkWorker = new NetworkWorker(m_networkModel);
    m_listModel = new NetworkListModel(m_controlPanel);
    m_controlPanel->setModel(m_listModel);

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

    connect(m_delayRefreshTimer, &QTimer::timeout, this, &NetworkPlugin::refreshWiredItemVisible);
    connect(m_networkModel, &NetworkModel::deviceListChanged, this, &NetworkPlugin::onDeviceListChanged);
    connect(m_networkModel, &NetworkModel::connectivityChanged, this, &NetworkPlugin::onConnectivityChanged);

    m_networkWorker->active();
    m_proxyInter->addItem(this, "network");

    PluginsItem* item = ((MainPanel*)m_proxyInter)->getItem(this, "network");
    connect(m_controlPanel, &NetworkControlPanel::sizeChanged, item, &PluginsItem::resizePopupWindow);

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

    m_listModel->setDeviceList(m_itemsMap);

    m_delayRefreshTimer->start();
}


void NetworkPlugin::onConnectivityChanged(Connectivity connectivity)
{
    if(connectivity == Connectivity::NoConnectivity || connectivity == Connectivity::UnknownConnectivity)
    {
        qDebug("connectivity 1");
        m_networkWidget->setIcon(DHiDPIHelper::loadNxPixmap(QString(":/wireless/resources/wireless/wireless-disconnect-symbolic.svg")));
    } 
    else if(connectivity == Connectivity::Limited) 
    {
        qDebug("connectivity 2");
        m_networkWidget->setIcon(DHiDPIHelper::loadNxPixmap(QString(":/wireless/resources/wireless/wireless-2-symbolic.svg")));
    }
    else if(connectivity == Connectivity::Limited)
    {
        qDebug("connectivity 3");
        m_networkWidget->setIcon(DHiDPIHelper::loadNxPixmap(QString(":/wireless/resources/wireless/wireless-8-symbolic.svg")));
    }
}
