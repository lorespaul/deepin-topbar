#include "networkcontrolpanel.h"
#include "networklistmodel.h"
#include "networklistview.h"

using namespace dtb;
using namespace dtb::network;

NetworkControlPanel::NetworkControlPanel(NetworkPlugin *networkPlugin, NetworkWorker *networkWorker, NetworkListModel *model, QWidget *parent)
    : QWidget(parent)
    , m_layout(new QVBoxLayout)
    , m_listView(new NetworkListView)
    , m_networkPlugin(networkPlugin)
    , m_networkWorker(networkWorker)
    , m_listModel(model)
{
    m_layout->setMargin(0);
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_listView);
    
    setLayout(m_layout);

    m_listView->setModel(m_listModel);

    connect(m_listModel, &NetworkListModel::layoutChanged, this, &NetworkControlPanel::adjustWidgetSize);

    connect(m_listView, &NetworkListView::entered, model, &NetworkListModel::setHoverIndex);
    // connect(m_listView, &NetworkListView::leaveEvent, model, &NetworkListModel::removeHoverIndex);


    connect(m_listView, &NetworkListView::clicked, this, [=](const QModelIndex &index){
        NetworkDevice *networkDevice = m_listModel->getCurrentNetworkDevice();

        if(networkDevice->type() == NetworkDevice::Wireless){
            
            WirelessDevice *device = (WirelessDevice*)networkDevice;
            AccessPoint *ap = m_listModel->getAP(index);
            QString ssid = ap->ssid();
            m_networkPlugin->connectingSsid = ssid;
            m_networkPlugin->startConnectingTimer();
            QString uuid;
            QList<QJsonObject> connections = device->connections();
            for (auto item : connections) {
                if (item.value("Ssid").toString() != ssid)
                    continue;

                uuid = item.value("Uuid").toString();
                if (!uuid.isEmpty())
                    break;
            }

            m_networkWorker->activateAccessPoint(device->path(), ap->path(), uuid);
        }
    });

    //TODO: add wifi on/off + refresh aps controls
}


void NetworkControlPanel::adjustWidgetSize()
{
    resize(360, m_listModel->rowCount() * 36);
    emit sizeChanged(360, m_listModel->rowCount() * 36);
}