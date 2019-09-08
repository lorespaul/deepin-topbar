#ifndef NETWORKDELEGATE_H
#define NETWORKDELEGATE_H

#include <QAbstractItemDelegate>

namespace dtb {
namespace network {
class NetworkDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    explicit NetworkDelegate(QObject *parent = nullptr);

    virtual void paint(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;

    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const;

private:
    virtual void drawPixmap(QPainter *painter, QString path, QSize size, QPoint point) const;

};
}
}

#endif // NETWORKDELEGATE_H
