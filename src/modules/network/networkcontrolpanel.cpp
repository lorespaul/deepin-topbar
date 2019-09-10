#include "networkcontrolpanel.h"
#include "networklistmodel.h"
#include "networklistview.h"
#include "frame/mainframe.h"
#include "widgets/dwidgetaction.h"
#include "widgets/switchitem.h"

#define CONTROLS_MENU_HEIGHT 39

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

    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    hBoxLayout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    hBoxLayout->setContentsMargins(20, 4, 5, 4);

    QLabel *networkLbl = new QLabel;
    networkLbl->setText(tr("Wireless Network"));
    networkLbl->setStyleSheet("color: white;");

    SwitchItem *switchItem = new SwitchItem;
    QTimer::singleShot(1000, this, [=]{
        // wait for device ready
        switchItem->setCheck(m_listModel->getCurrentNetworkDevice()->enabled());
    });
    switchItem->setContentsMargins(7, 0, 0, 0);
    connect(switchItem, &SwitchItem::clicked, this, [=] (const bool state) {
        m_networkWorker->setDeviceEnable(m_listModel->getCurrentNetworkDevice()->path(), state);
        m_listModel->toggleList(state);
        emit m_listModel->layoutChanged();
    });

    hBoxLayout->addWidget(networkLbl);
    hBoxLayout->addWidget(switchItem);

    QVBoxLayout *vBoxLayout = new QVBoxLayout;
    vBoxLayout->setMargin(0);
    vBoxLayout->setSpacing(0);
    vBoxLayout->setContentsMargins(0, 0, 0, 0);
    vBoxLayout->addWidget(m_listView);

    QSize size = m_listModel->data(m_listModel->index(0), NetworkListModel::SizeRole).toSize();
    QWidget *spacer = new QWidget;
    spacer->setContentsMargins(0, 0, 0, 0);
    spacer->setFixedHeight(1);
    spacer->setStyleSheet("background-color: rgba(200, 200, 200, 150)");

    m_layout->addLayout(hBoxLayout);
    m_layout->addWidget(spacer);
    m_layout->addLayout(vBoxLayout);
    setLayout(m_layout);
    setStyleSheet("border: none");

    resize(size.width(), CONTROLS_MENU_HEIGHT);

    m_listView->setModel(m_listModel);



    connect(m_listModel, &NetworkListModel::layoutChanged, this, &NetworkControlPanel::adjustWidgetSize);

    connect(m_listView, &NetworkListView::entered, model, &NetworkListModel::setHoverIndex);
    connect(m_listView, &NetworkListView::exited, model, &NetworkListModel::removeHoverIndex);

    connect(m_listView, &NetworkListView::clicked, this, [=](const QModelIndex &index){
        NetworkDevice *networkDevice = m_listModel->getCurrentNetworkDevice();

        if(networkDevice->type() == NetworkDevice::Wireless){
            
            WirelessDevice *device = (WirelessDevice*)networkDevice;
            AccessPoint *ap = m_listModel->getAP(index);
            if(ap == nullptr) return;
            QString ssid = ap->ssid();

            if(device->activeApSsid() == ssid)
                return; // not connect to already connected ap

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

}


void NetworkControlPanel::adjustWidgetSize()
{
    QSize size = m_listModel->data(m_listModel->index(0), NetworkListModel::SizeRole).toSize();
    resize(size.width(), (m_listModel->rowCount(m_listModel->index(-1)) * size.height()) + CONTROLS_MENU_HEIGHT);
    emit sizeChanged();
}