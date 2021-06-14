#include "ibeduploadertool.h"
#include "ibeduploader.h"
#include <QPainter>

IBedUploaderTool::IBedUploaderTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool IBedUploaderTool::closeOnButtonPressed() const
{
    return true;
}

QIcon IBedUploaderTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "cloud-upload.svg");
}
QString IBedUploaderTool::name() const
{
    return tr("Image Uploader BED");
}

ToolType IBedUploaderTool::nameID() const
{
    return ToolType::UPLOADBED;
}

QString IBedUploaderTool::description() const
{
    return tr("Upload the selection to Bed");
}

QWidget* IBedUploaderTool::widget()
{
    return new IBedUploader(capture);
}

CaptureTool* IBedUploaderTool::copy(QObject* parent)
{
    return new IBedUploaderTool(parent);
}

void IBedUploaderTool::pressed(const CaptureContext& context)
{
    emit requestAction(REQ_CLEAR_SELECTION);
    capture = context.selectedScreenshotArea();
    emit requestAction(REQ_CAPTURE_DONE_OK);
    emit requestAction(REQ_ADD_EXTERNAL_WIDGETS);
}
