#pragma once

#include "src/tools/abstractactiontool.h"

class IBedUploaderTool : public AbstractActionTool
{
    Q_OBJECT
public:
    explicit IBedUploaderTool(QObject* parent = nullptr);

    bool closeOnButtonPressed() const override;

    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    QString description() const override;

    QWidget* widget() override;

    CaptureTool* copy(QObject* parent = nullptr) override;

protected:
    ToolType nameID() const override;

public slots:
    void pressed(const CaptureContext& context) override;

private:
    QPixmap capture;
};
