#include "notifyplugin.h"
#include "notifywidget.h"

#include <DHiDPIHelper>
#include <QApplication>

using namespace dtb;
using namespace dtb::notify;

DWIDGET_USE_NAMESPACE

NotifyPlugin::NotifyPlugin()
{
    // m_notify = new NotifyWidget;
    // m_notify->setFixedSize(360, 720 / qApp->devicePixelRatio());
    m_notifyBtn = new QPushButton;
    m_notifyBtn->setContentsMargins(0, 0, 0, 0);
    m_notifyBtn->setObjectName("NotifyButton");
    m_notifyBtn->setIcon(DHiDPIHelper::loadNxPixmap(":/images/split_normal.svg"));
    m_notifyBtn->setIconSize(QSize(30, 30));
    m_notifyBtn->setStyleSheet("QPushButton#NotifyButton {background-color: transparent; border: none;}");
    
    connect(m_notifyBtn, SIGNAL(clicked()), this, SLOT(openNoifications()));
}

const QString NotifyPlugin::pluginName() const
{
    return "Notify";
}

void NotifyPlugin::openNoifications() 
{
    QProcess::startDetached("dbus-send --print-reply --dest=com.deepin.dde.ControlCenter /com/deepin/dde/ControlCenter com.deepin.dde.ControlCenter.ShowNotifications");
}

void NotifyPlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;

    m_proxyInter->addItem(this, "");
}

QWidget *NotifyPlugin::itemWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    return m_notifyBtn;
}

// QWidget *NotifyPlugin::itemContextMenu(const QString &itemKey)
// {
//     Q_UNUSED(itemKey);

//     return m_notify;
// }

void NotifyPlugin::setDefaultColor(PluginProxyInterface::DefaultColor color)
{
    Q_UNUSED(color);
}
