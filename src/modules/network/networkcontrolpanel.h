#ifndef NETWORKCONTROLPANEL_H
#define NETWORKCONTROLPANEL_H

#include <NetworkWorker>

#include <QWidget>
#include <QVBoxLayout>

using namespace dde::network;

namespace dtb {
namespace network {
class NetworkListView;
class NetworkListModel;
class NetworkControlPanel : public QWidget
{
    Q_OBJECT
public:
    explicit NetworkControlPanel(NetworkWorker *networkWorker, NetworkListModel *model, QWidget *parent = nullptr);

signals:
    void sizeChanged(int width, int height);

public slots:
    void adjustWidgetSize();

private:
    QVBoxLayout *m_layout;
    NetworkListView *m_listView;
    NetworkListModel * const m_listModel;
    NetworkWorker * const m_networkWorker;
};
}
}

#endif // NETWORKCONTROLPANEL_H
