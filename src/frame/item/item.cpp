#include "item.h"
#include <QDebug>

using namespace dtb;

Item::Item(QWidget *parent)
    : QWidget(parent)
{
    PopupWindow = new ItemPopupWindow(nullptr);
}

PluginsItemInterface *Item::itemInter()
{
    return nullptr;
}

QWidget *Item::contextMenu() const
{
    return nullptr;
}


void Item::hidePopupWindow()
{
    PopupWindow->hide();
}
