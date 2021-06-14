#pragma once

#include "visualseditor.h"

#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class ExtendedSlider;
class QVBoxLayout;
class ButtonListView;
class UIcolorEditor;

class IbedConf : public QWidget
{
    Q_OBJECT
public:
    explicit IbedConf(QWidget* parent = nullptr);

public slots:
    void updateComponents();

private slots:

private:
    QVBoxLayout* m_layout;
    QLineEdit* m_username;
    QPushButton* m_saveButton;
    QPushButton* m_resetButton;
    QPushButton* m_clearButton;
    void initLayout();
    void initWidgets();
    void saveIBed();
    void resetName();
};
