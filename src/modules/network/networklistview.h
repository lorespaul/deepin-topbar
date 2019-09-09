#ifndef NETWORKLISTVIEW_H
#define NETWORKLISTVIEW_H

#include <QListView>

namespace dtb {
namespace network {
class NetworkListView : public QListView
{
    Q_OBJECT
public:
    explicit NetworkListView(QWidget *parent = nullptr);
    void leaveEvent(QEvent *event) override;

signals:
    void exited();

public slots:
};
}
}

#endif // NETWORKLISTVIEW_H
