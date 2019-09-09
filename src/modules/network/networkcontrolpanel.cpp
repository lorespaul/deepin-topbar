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
    setStyleSheet("border: none");

    m_listView->setModel(m_listModel);

    connect(m_listModel, &NetworkListModel::layoutChanged, this, &NetworkControlPanel::adjustWidgetSize);

    connect(m_listView, &NetworkListView::entered, model, &NetworkListModel::setHoverIndex);
    connect(m_listView, &NetworkListView::exited, model, &NetworkListModel::removeHoverIndex);

    connect(m_listView, &NetworkListView::clicked, this, [=](const QModelIndex &index){
        NetworkDevice *networkDevice = m_listModel->getCurrentNetworkDevice();

        if(networkDevice->type() == NetworkDevice::Wireless){
            
            WirelessDevice *device = (WirelessDevice*)networkDevice;
            AccessPoint *ap = m_listModel->getAP(index);
            QString ssid = ap->ssid();
            m_networkPlugin->startConnectingTimer(ssid);
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
    QSize size = m_listModel->data(m_listModel->index(0), NetworkListModel::SizeRole).toSize();
    resize(size.width(), m_listModel->rowCount() * size.height());
    emit sizeChanged();
}