#include "networkdelegate.h"

#include "networklistmodel.h"

#include <QPainter>
#include <DHiDPIHelper>
#include <QApplication>
#include <QImageReader>
#include <QPushButton>

DWIDGET_USE_NAMESPACE

using namespace dtb;
using namespace dtb::network;

NetworkDelegate::NetworkDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{

}

void NetworkDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QString name { index.data(NetworkListModel::NameRole).toString() };
    const QString icon { index.data(NetworkListModel::IconRole).toString() };
    const bool isSecurity { index.data(NetworkListModel::SecurityRole).toBool() };

    QPoint rightIconPoint(option.rect.topRight());
    rightIconPoint.setX(rightIconPoint.x() - 23);
    rightIconPoint.setY(rightIconPoint.y() + 6);

    if (index.data(NetworkListModel::HoverRole).toBool()) {
        painter->fillRect(option.rect, QColor(16, 109, 150, 150));

        if(index.data(NetworkListModel::ActiveRole).toBool()){
            drawPixmap(painter, ":/wireless/resources/wireless/disconnect.png", QSize(14, 14), rightIconPoint);
        }

    } else if(!index.data(NetworkListModel::HoverRole).toBool() && index.data(NetworkListModel::ActiveRole).toBool()){

        drawPixmap(painter, ":/wireless/resources/wireless/selected.png", QSize(14, 14), rightIconPoint);
    }

    QPoint cachedTopLeft = QPoint(option.rect.topLeft());

    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    const QPixmap pixmap = DHiDPIHelper::loadNxPixmap(icon).scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPoint wiconPoint = option.rect.topLeft();
    wiconPoint.setX(wiconPoint.x() + 20);
    QRect pixRect(wiconPoint, pixmap.size());
    painter->drawPixmap(pixRect, pixmap);

    QPoint textPoint = pixRect.topRight();
    textPoint.setX(textPoint.x() + 5);
    textPoint.setY(textPoint.y() + 7);
    QRect textRect(textPoint, QSize(option.fontMetrics.width(name) + 10, option.fontMetrics.height()));
    painter->setPen(Qt::white);
    painter->drawText(textRect, name);

    if (isSecurity)
        drawPixmap(painter, ":/wireless/resources/wireless/security.svg", QSize(23, 23), cachedTopLeft);
}


void NetworkDelegate::drawPixmap(QPainter *painter, QString path, QSize size, QPoint point) const
{
    QPixmap pixmap;
    QImageReader reader;
    qreal sourceDevicePixelRatio = 1.0;
    qreal devicePixelRatio = qApp->devicePixelRatio();
    reader.setFileName(path);
    if (reader.canRead()) {
        pixmap = QPixmap::fromImage(reader.read())
                    .scaled(size * (devicePixelRatio / sourceDevicePixelRatio), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        pixmap.setDevicePixelRatio(devicePixelRatio);
    }
    QRect rect(QPoint(point.x(), point.y() + 4), pixmap.size() / qApp->devicePixelRatio());
    painter->drawPixmap(rect, pixmap);
}


bool NetworkDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if(event->type() == QEvent::MouseButtonRelease && index.data(NetworkListModel::ActiveRole).toBool()){
        QString ssid = index.data(NetworkListModel::NameRole).toString();
        emit ((NetworkListModel*)index.model())->deactivateActiveConnection(ssid);
    }
    return false;
}



QSize NetworkDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return index.data(NetworkListModel::SizeRole).toSize();
}
