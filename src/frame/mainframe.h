#ifndef MAINFRAME_H
#define MAINFRAME_H

#define QMENU_STYLE "QMenu{background-color: transparent; margin: 0px;}QMenu::item{color: #d3d3d3; margin: 0px; padding: 3px 18px;}QMenu::item:selected{background: rgba(16, 109, 170, 150);}QMenu::separator{height: 1px;background: rgba(200, 200, 200, 150);}"

#include <QFrame>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QDesktopWidget>
#include <DBlurEffectWidget>
#include <QPropertyAnimation>
#include <DPlatformWindowHandle>
#include <QPropertyAnimation>
#include <DForeignWindow>
#include <com_deepin_dde_daemon_dock.h>
#include <com_deepin_daemon_appearance.h>
#include <memory>

#include "mainpanel.h"

using DockInter = com::deepin::dde::daemon::Dock;
using Appearance = com::deepin::daemon::Appearance;

DWIDGET_USE_NAMESPACE

class MainFrame : public DBlurEffectWidget
{
    Q_OBJECT
public:
    explicit MainFrame(QWidget *parent = 0);
    ~MainFrame();

public slots:
    void showSetting();

private slots:
    void screenChanged();
#if (DTK_VERSION >= DTK_VERSION_CHECK(2, 0, 9, 10))
    void onWindowListChanged();
#endif
    void onWindowStateChanged(Qt::WindowState windowState);
    void delayedScreenChanged();
    void opacityChanged(double opacity);

private:
    void init();
    void initConnect();
    void initAnimation();
    void onWindowPosChanged(DForeignWindow *window);
    void updateBackground();
    void updateBorderPath();
    void setBackground(const QColor &color);
    void onBackgroundChanged(const QColor &color);
    QPainterPath pathHandle() const;

#ifdef ENABLE_RATOTE
    bool isRotated() const;
#endif

private:
    QDesktopWidget *m_desktopWidget;
    dtb::MainPanel *m_mainPanel;
    DPlatformWindowHandle *m_handle;
    QPropertyAnimation *m_launchAni;
//    QPropertyAnimation *m_hideWithLauncher;
//    QPropertyAnimation *m_showWithLauncher;
    DockInter *m_dockInter;
    Appearance *m_appearanceInter;
    QWidget *m_structWidget;

    QMap<WId,DForeignWindow*> m_windowList;
    QList<WId> m_windowIdList;
    QList<WId> m_maxWindowList;
    QList<WId> m_overlapping;
    QList<WId> m_fullWindowList;
    QVariantAnimation *m_backgroundAni;
    QColor m_defaultBgColor;
    QColor m_backgroundColor;
};

#endif // MAINFRAME_H
