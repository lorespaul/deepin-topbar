#include "widgets/fontlabel.h"

#include <QApplication>
#include <QFile>
#include <QFont>
#include <QFontDatabase>

using namespace dtb::widgets;

FontLabel::FontLabel(QWidget *parent) : QLabel(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setObjectName("FontLabel");

    int fontId = QFontDatabase::addApplicationFont(":/segoe-mdl2-assets.ttf");
    QString fontName=QFontDatabase::applicationFontFamilies(fontId).at(0);

    m_iconFont = QFont(fontName);
}

void FontLabel::setIcon(const QChar c, int size)
{
    m_iconFont.setPointSize(size);
    setFont(m_iconFont);
    setText(c);
}

void FontLabel::setIcon(QPixmap pixmap)
{
    pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
    setPixmap(pixmap);
    setContentsMargins(5, 0, 5, 0);
}

void FontLabel::mouseReleaseEvent(QMouseEvent *event)
{
    QLabel::mouseReleaseEvent(event);

    emit clicked();
}
