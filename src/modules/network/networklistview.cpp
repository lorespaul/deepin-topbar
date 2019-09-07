#include "networklistview.h"
#include "networkdelegate.h"

using namespace dtb;
using namespace dtb::network;

NetworkListView::NetworkListView(QWidget *parent)
    : QListView(parent)
{
    setMouseTracking(true);
    setItemDelegate(new NetworkDelegate);

    setContentsMargins(0, 0, 0, 0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
    setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
}
