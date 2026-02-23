#include "NodeItem.h"
#include "EdgeItem.h"
#include "canvas/SpatialScene.h"
#include "layout/LayoutEngine.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QGuiApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QInputDialog>
#include <QFile>
#include <QDir>
#include <QSvgRenderer>
#include <QFileIconProvider>

NodeItem::NodeItem(const QFileInfo &fileInfo, QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , m_fileInfo(fileInfo)
{
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
    setCacheMode(NoCache);  // Dimming depends on external state
    setZValue(10);

    if (fileInfo.absoluteFilePath().isEmpty()) {
        // Placeholder / load-more node
        m_filePath = "";
        m_fileName = "";
        m_isDir = false;
        return;
    }

    m_filePath = fileInfo.absoluteFilePath();
    m_fileName = fileInfo.fileName();
    m_label = m_fileName;
    m_isDir = fileInfo.isDir();
    m_fileSize = fileInfo.size();
    m_isSymlink = fileInfo.isSymLink();

    if (m_isSymlink) {
        m_symlinkTarget = fileInfo.symLinkTarget();
    }

    // If the filename is empty (root drive), use the path
    if (m_fileName.isEmpty()) {
        m_fileName = m_filePath;
        m_label = m_filePath;
    }
}

NodeItem::~NodeItem()
{
}

QRectF NodeItem::boundingRect() const
{
    return QRectF(-NODE_WIDTH / 2, -NODE_HEIGHT / 2, NODE_WIDTH, NODE_HEIGHT);
}

QPainterPath NodeItem::shape() const
{
    QPainterPath path;
    path.addRoundedRect(boundingRect(), 10, 10);
    return path;
}

void NodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    qreal lod = option->levelOfDetailFromTransform(painter->worldTransform());
    paintNode(painter, lod);
}

void NodeItem::paintNode(QPainter *painter, qreal lod)
{
    painter->setRenderHint(QPainter::Antialiasing);

    QRectF rect = boundingRect();
    QColor bgColor = nodeColor();

    // Check if this node is in the active (most recently expanded) branch
    bool isDimmed = false;
    if (m_spatialScene) {
        isDimmed = !m_spatialScene->isInActiveBranch(this);
    }

    // Hover/selection effects
    if (isSelected()) {
        bgColor = bgColor.lighter(130);
    }
    if (m_isHovered) {
        bgColor = bgColor.lighter(115);
        isDimmed = false;  // Always bright on hover
    }

    // Apply dimming: darken color and reduce overall opacity
    qreal dimOpacity = isDimmed ? 0.35 : 1.0;
    if (isDimmed) {
        bgColor = bgColor.darker(180);
    }
    painter->setOpacity(dimOpacity);

    // At very low LOD, just draw a colored dot
    if (lod < 0.2) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(bgColor);
        painter->drawEllipse(QPointF(0, 0), 6, 6);
        painter->setOpacity(1.0);
        return;
    }

    // Draw rounded rectangle background
    QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
    gradient.setColorAt(0, bgColor.lighter(110));
    gradient.setColorAt(1, bgColor);

    painter->setPen(QPen(bgColor.darker(150), 1.5));
    painter->setBrush(gradient);
    painter->drawRoundedRect(rect, 10, 10);

    // Blue selection border
    if (isSelected()) {
        painter->setPen(QPen(QColor(70, 140, 220), 2.0));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(rect.adjusted(1, 1, -1, -1), 9, 9);
    }

    // Draw expand indicator for folders
    if (m_isDir && !m_isLoadMore) {
        QRectF indicatorRect(rect.right() - 18, rect.top() + 4, 14, 14);
        painter->setPen(QPen(QColor(180, 180, 180), 1.5));
        painter->setBrush(Qt::NoBrush);
        if (m_isExpanded) {
            // Down chevron
            painter->drawLine(QPointF(indicatorRect.left() + 2, indicatorRect.center().y() - 2),
                            QPointF(indicatorRect.center().x(), indicatorRect.center().y() + 3));
            painter->drawLine(QPointF(indicatorRect.center().x(), indicatorRect.center().y() + 3),
                            QPointF(indicatorRect.right() - 2, indicatorRect.center().y() - 2));
        } else {
            // Right chevron
            painter->drawLine(QPointF(indicatorRect.center().x() - 2, indicatorRect.top() + 2),
                            QPointF(indicatorRect.center().x() + 3, indicatorRect.center().y()));
            painter->drawLine(QPointF(indicatorRect.center().x() + 3, indicatorRect.center().y()),
                            QPointF(indicatorRect.center().x() - 2, indicatorRect.bottom() - 2));
        }
    }

    // At medium LOD, skip text
    if (lod < 0.5) return;

    // Draw icon area
    QRectF iconRect(rect.left() + 8, rect.top() + 10, 28, 28);
    paintIcon(painter, iconRect);

    // Draw label
    QRectF textRect(rect.left() + 42, rect.top() + 6, rect.width() - 60, 22);
    painter->setPen(QColor(230, 230, 230));
    QFont font("Segoe UI", 9);
    font.setWeight(QFont::Medium);
    painter->setFont(font);

    QString elidedLabel = painter->fontMetrics().elidedText(m_label, Qt::ElideMiddle, textRect.width());
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, elidedLabel);

    // Draw sublabel (file size or child count hint)
    if (lod > 0.8) {
        QRectF subRect(rect.left() + 42, rect.top() + 28, rect.width() - 60, 18);
        painter->setPen(QColor(140, 140, 140));
        QFont subFont("Segoe UI", 7);
        painter->setFont(subFont);

        QString subLabel;
        if (m_isLoadMore) {
            subLabel = "Click to load";
        } else if (m_permissionDenied) {
            subLabel = "Access denied";
        } else if (m_isSymlink) {
            subLabel = "Symlink";
        } else if (m_isDrive) {
            subLabel = "Drive";
        } else if (m_isDir) {
            subLabel = m_isExpanded ? "Expanded" : "Folder";
        } else {
            subLabel = fileTypeLabel();
            if (m_fileSize > 0) {
                if (m_fileSize < 1024)
                    subLabel += QString(" · %1 B").arg(m_fileSize);
                else if (m_fileSize < 1024 * 1024)
                    subLabel += QString(" · %1 KB").arg(m_fileSize / 1024);
                else if (m_fileSize < 1024LL * 1024 * 1024)
                    subLabel += QString(" · %1 MB").arg(m_fileSize / (1024 * 1024));
                else
                    subLabel += QString(" · %1 GB").arg(m_fileSize / (1024LL * 1024 * 1024));
            }
        }
        painter->drawText(subRect, Qt::AlignLeft | Qt::AlignVCenter, subLabel);
    }

    // Lock overlay
    if (m_permissionDenied) {
        painter->setPen(QPen(QColor(239, 83, 80, 180), 2));
        painter->setBrush(QColor(239, 83, 80, 40));
        painter->drawRoundedRect(rect.adjusted(2, 2, -2, -2), 8, 8);
    }

    // Reset opacity
    painter->setOpacity(1.0);
}

void NodeItem::paintIcon(QPainter *painter, const QRectF &iconRect)
{
    QString iconPath;
    if (m_isLoadMore)
        iconPath = ":/icons/loadmore.svg";
    else if (m_permissionDenied)
        iconPath = ":/icons/lock.svg";
    else if (m_isSymlink)
        iconPath = ":/icons/symlink.svg";
    else if (m_isDrive)
        iconPath = ":/icons/drive.svg";
    else if (m_isDir)
        iconPath = ":/icons/folder.svg";
    else
        iconPath = ":/icons/file.svg";

    QSvgRenderer renderer(iconPath);
    if (renderer.isValid()) {
        renderer.render(painter, iconRect);
    } else {
        // Fallback: simple colored rectangle
        painter->setPen(Qt::NoPen);
        painter->setBrush(m_isDir ? QColor(248, 216, 120) : QColor(200, 210, 220));
        painter->drawRoundedRect(iconRect, 4, 4);
    }
}

QColor NodeItem::nodeColor() const
{
    if (m_isLoadMore)     return QColor(55, 65, 80);
    if (m_permissionDenied) return QColor(80, 40, 40);
    if (m_isDrive)         return QColor(45, 55, 75);
    if (m_isSymlink)       return QColor(50, 60, 70);
    if (m_isDir) {
        if (m_isExpanded)  return QColor(50, 65, 50);
        return QColor(55, 60, 45);
    }

    // File type colors
    QString ext = m_fileInfo.suffix().toLower();
    if (ext == "pdf" || ext == "doc" || ext == "docx" || ext == "txt")
        return QColor(60, 50, 70);
    if (ext == "jpg" || ext == "png" || ext == "svg" || ext == "gif" || ext == "bmp" || ext == "webp")
        return QColor(50, 60, 70);
    if (ext == "mp4" || ext == "avi" || ext == "mkv" || ext == "mov" || ext == "wmv")
        return QColor(70, 50, 55);
    if (ext == "mp3" || ext == "wav" || ext == "flac" || ext == "ogg")
        return QColor(60, 55, 70);
    if (ext == "exe" || ext == "msi")
        return QColor(70, 55, 45);
    if (ext == "zip" || ext == "rar" || ext == "7z" || ext == "tar" || ext == "gz")
        return QColor(55, 65, 55);
    if (ext == "cpp" || ext == "h" || ext == "py" || ext == "js" || ext == "ts" || ext == "java" || ext == "cs")
        return QColor(45, 60, 75);

    return QColor(50, 52, 58);
}

QString NodeItem::fileTypeLabel() const
{
    if (m_isDir || m_isDrive) return "";
    QString ext = m_fileInfo.suffix().toUpper();
    return ext.isEmpty() ? "File" : ext;
}

void NodeItem::setExpanded(bool expanded)
{
    m_isExpanded = expanded;
    update();
}

void NodeItem::setPermissionDenied(bool denied)
{
    m_permissionDenied = denied;
    update();
}

void NodeItem::addChild(NodeItem *child)
{
    child->setParentNode(this);
    m_children.append(child);
}

void NodeItem::removeChild(NodeItem *child)
{
    m_children.removeOne(child);
    child->setParentNode(nullptr);
}

void NodeItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        emit clicked(this);

        if (m_spatialScene) {
            emit m_spatialScene->nodeSelected(m_filePath);
        }
    }
    QGraphicsObject::mousePressEvent(event);
}

void NodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_isDragging = false;
    QGraphicsObject::mouseReleaseEvent(event);
}

void NodeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_isLoadMore && m_spatialScene) {
            // Fetch next batch
            NodeItem *parentNode = m_spatialScene->nodeForPath(m_parentPath);
            if (parentNode) {
                // Remove this load-more node and fetch again
                parentNode->removeChild(this);
                m_spatialScene->removeItem(this);
                // The worker will handle pagination
            }
            return;
        }

        if (m_isDir) {
            // Toggle expand/collapse
            if (m_spatialScene) {
                if (m_isExpanded) {
                    m_spatialScene->collapseNode(this);
                } else {
                    m_spatialScene->expandNode(this);
                }
            }
        } else {
            // Open file with default OS handler
            QDesktopServices::openUrl(QUrl::fromLocalFile(m_filePath));
        }
    }
    QGraphicsObject::mouseDoubleClickEvent(event);
}

void NodeItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    menu.setStyleSheet(
        "QMenu { background: #2a2a2a; border: 1px solid #4a4a4a; color: #ddd; padding: 4px; }"
        "QMenu::item { padding: 6px 24px; }"
        "QMenu::item:selected { background: #3a5a8a; }"
        "QMenu::separator { background: #4a4a4a; height: 1px; margin: 4px 8px; }"
    );

    if (m_isDir) {
        QAction *openAction = menu.addAction("📂 Expand / Collapse");
        connect(openAction, &QAction::triggered, this, [this]() {
            if (m_spatialScene) {
                if (m_isExpanded) m_spatialScene->collapseNode(this);
                else m_spatialScene->expandNode(this);
            }
        });
        menu.addSeparator();
    }

    if (!m_isDir && !m_isLoadMore) {
        QAction *openAction = menu.addAction("▶ Open");
        connect(openAction, &QAction::triggered, this, [this]() {
            QDesktopServices::openUrl(QUrl::fromLocalFile(m_filePath));
        });
    }

    QAction *explorerAction = menu.addAction("📁 Show in Explorer");
    connect(explorerAction, &QAction::triggered, this, [this]() {
        QProcess::startDetached("explorer", {"/select,", QDir::toNativeSeparators(m_filePath)});
    });

    menu.addSeparator();

    QAction *copyPathAction = menu.addAction("📋 Copy Path");
    connect(copyPathAction, &QAction::triggered, this, [this]() {
        QGuiApplication::clipboard()->setText(QDir::toNativeSeparators(m_filePath));
    });

    QAction *renameAction = menu.addAction("✏️ Rename");
    connect(renameAction, &QAction::triggered, this, [this]() {
        bool ok;
        QString newName = QInputDialog::getText(nullptr, "Rename", "New name:",
                                                 QLineEdit::Normal, m_fileName, &ok);
        if (ok && !newName.isEmpty() && newName != m_fileName) {
            QString dir = QFileInfo(m_filePath).absolutePath();
            QString newPath = dir + "/" + newName;
            if (QFile::rename(m_filePath, newPath)) {
                m_filePath = newPath;
                m_fileName = newName;
                m_label = newName;
                m_fileInfo = QFileInfo(newPath);
                update();
            }
        }
    });

    menu.addSeparator();

    QAction *deleteAction = menu.addAction("🗑️ Delete");
    connect(deleteAction, &QAction::triggered, this, [this]() {
        auto result = QMessageBox::question(nullptr, "Delete",
            QString("Move '%1' to trash?").arg(m_fileName),
            QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::Yes) {
            QFile::moveToTrash(m_filePath);
        }
    });

    menu.addSeparator();

    QAction *propsAction = menu.addAction("ℹ️ Properties");
    connect(propsAction, &QAction::triggered, this, [this]() {
        QString info;
        info += "Name: " + m_fileName + "\n";
        info += "Path: " + m_filePath + "\n";
        info += "Type: " + (m_isDir ? "Directory" : fileTypeLabel()) + "\n";
        if (!m_isDir) {
            info += "Size: " + QString::number(m_fileSize) + " bytes\n";
        }
        info += "Modified: " + m_fileInfo.lastModified().toString(Qt::ISODate) + "\n";
        if (m_isSymlink) {
            info += "Symlink target: " + m_symlinkTarget + "\n";
        }
        info += "Readable: " + QString(m_fileInfo.isReadable() ? "Yes" : "No") + "\n";
        info += "Writable: " + QString(m_fileInfo.isWritable() ? "Yes" : "No") + "\n";
        QMessageBox::information(nullptr, "Properties: " + m_fileName, info);
    });

    menu.exec(event->screenPos());
}

void NodeItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    m_isHovered = true;
    setZValue(20);
    update();
}

void NodeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    m_isHovered = false;
    setZValue(10);
    update();
}

QVariant NodeItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange) {
        m_lastPos = pos();
    }
    if (change == ItemPositionHasChanged) {
        // Move descendant nodes
        if (!m_movingChildren) {
            QPointF delta = pos() - m_lastPos;
            if (!delta.isNull()) {
                m_movingChildren = true;
                if (m_isDragging && m_isExpanded && m_spatialScene) {
                    // User is dragging this expanded node:
                    // re-layout direct children around new position
                    m_spatialScene->layoutEngine()->relayoutAroundParent(this, delta);
                } else {
                    // Programmatic move or non-expanded: rigid translation
                    moveChildrenBy(delta);
                }
                m_movingChildren = false;
            }
        }
        // Update connected edges
        if (m_edgeToParent)
            m_edgeToParent->updatePosition();
        for (NodeItem *child : m_children) {
            if (child->edgeToParent())
                child->edgeToParent()->updatePosition();
        }
    }
    return QGraphicsObject::itemChange(change, value);
}

void NodeItem::moveChildrenBy(const QPointF &delta)
{
    for (NodeItem *child : m_children) {
        child->m_movingChildren = true;
        child->moveBy(delta.x(), delta.y());
        child->m_movingChildren = false;
        child->moveChildrenBy(delta);
    }
}
