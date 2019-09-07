#include "networkcontrolpanel.h"
#include "networklistmodel.h"
#include "networklistview.h"

using namespace dtb;
using namespace dtb::network;

NetworkControlPanel::NetworkControlPanel(NetworkWorker *networkWorker, NetworkListModel *model, QWidget *parent)
    : QWidget(parent)
    , m_layout(new QVBoxLayout)
    , m_listView(new NetworkListView)
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
    connect(m_listView, &NetworkListView::clicked, this, [=](const QModelIndex &index){
        AccessPoint *ap = m_listModel->getAP(index);
        m_networkWorker->activateAccessPoint(m_listModel->getCurrentNetworkDevice()->path(), ap->path(), ap->ssid());
        qDebug("connect to devPath " + m_listModel->getCurrentNetworkDevice()->path().toUtf8() + ", apPath " + ap->path().toUtf8() + ", ssid " + ap->ssid().toUtf8());
    });
}


void NetworkControlPanel::adjustWidgetSize()
{
    resize(360, m_listModel->rowCount() * 50);
    emit sizeChanged(360, m_listModel->rowCount() * 52);
}