#include "widget/wtrackproperty.h"

#include <QDebug>

#include "moc_wtrackproperty.cpp"
#include "skin/legacy/skincontext.h"
#include "track/track.h"
#include "util/dnd.h"
#include "widget/wtrackmenu.h"

WTrackProperty::WTrackProperty(
        QWidget* pParent,
        UserSettingsPointer pConfig,
        Library* pLibrary,
        const QString& group)
        : WLabel(pParent),
          m_group(group),
          m_pConfig(pConfig),
          m_pLibrary(pLibrary) {
    setAcceptDrops(true);
}

WTrackProperty::~WTrackProperty() {
    // Required to allow forward declaration of WTrackMenu in header
}

void WTrackProperty::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);

    QString property = context.selectString(node, "Property");
    if (property.isEmpty()) {
        return;
    }

    // Check if property with that name exists in Track class
    if (Track::staticMetaObject.indexOfProperty(property.toUtf8().constData()) == -1) {
        qWarning() << "WTrackProperty: Unknown track property:" << property;
        return;
    }
    m_property = property;
}

void WTrackProperty::slotTrackLoaded(TrackPointer pTrack) {
    if (!pTrack) {
        return;
    }
    m_pCurrentTrack = pTrack;
    connect(pTrack.get(),
            &Track::changed,
            this,
            &WTrackProperty::slotTrackChanged);
    updateLabel();
}

void WTrackProperty::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pNewTrack);
    Q_UNUSED(pOldTrack);
    if (m_pCurrentTrack) {
        disconnect(m_pCurrentTrack.get(), nullptr, this, nullptr);
    }
    m_pCurrentTrack.reset();
    updateLabel();
}

void WTrackProperty::slotTrackChanged(TrackId trackId) {
    Q_UNUSED(trackId);
    updateLabel();
}

void WTrackProperty::updateLabel() {
    if (m_pCurrentTrack) {
        if (m_property.isEmpty()) {
            return;
        }
        QVariant property =
                m_pCurrentTrack->property(m_property.toUtf8().constData());
        if (property.isValid() && property.canConvert<QString>()) {
            setText(property.toString());
            return;
        }
    }
    setText("");
}

void WTrackProperty::mousePressEvent(QMouseEvent* pEvent) {
    DragAndDropHelper::mousePressed(pEvent);
}

void WTrackProperty::mouseMoveEvent(QMouseEvent* pEvent) {
    if (m_pCurrentTrack && DragAndDropHelper::mouseMoveInitiatesDrag(pEvent)) {
        DragAndDropHelper::dragTrack(m_pCurrentTrack, this, m_group);
    }
}

void WTrackProperty::mouseDoubleClickEvent(QMouseEvent* pEvent) {
    Q_UNUSED(pEvent);
    if (!m_pCurrentTrack) {
        return;
    }
    ensureTrackMenuIsCreated();
    m_pTrackMenu->loadTrack(m_pCurrentTrack, m_group);
    m_pTrackMenu->showDlgTrackInfo(m_property);
}

void WTrackProperty::dragEnterEvent(QDragEnterEvent* pEvent) {
    DragAndDropHelper::handleTrackDragEnterEvent(pEvent, m_group, m_pConfig);
}

void WTrackProperty::dropEvent(QDropEvent* pEvent) {
    DragAndDropHelper::handleTrackDropEvent(pEvent, *this, m_group, m_pConfig);
}

void WTrackProperty::contextMenuEvent(QContextMenuEvent* pEvent) {
    pEvent->accept();
    if (m_pCurrentTrack) {
        ensureTrackMenuIsCreated();
        m_pTrackMenu->loadTrack(m_pCurrentTrack, m_group);
        // Show the right-click menu
        m_pTrackMenu->popup(pEvent->globalPos());
    }
}

void WTrackProperty::ensureTrackMenuIsCreated() {
    if (m_pTrackMenu.get() == nullptr) {
        m_pTrackMenu = make_parented<WTrackMenu>(
                this, m_pConfig, m_pLibrary, WTrackMenu::kDeckTrackMenuFeatures);
    }
    // Before and after the loaded tracks file has been removed from disk,
    // instruct the library to save and restore the current index for
    // keyboard/controller navigation.
    connect(m_pTrackMenu,
            &WTrackMenu::saveCurrentViewState,
            this,
            &WTrackProperty::saveCurrentViewState);
    connect(m_pTrackMenu,
            &WTrackMenu::restoreCurrentViewStateOrIndex,
            this,
            &WTrackProperty::restoreCurrentViewState);
}
