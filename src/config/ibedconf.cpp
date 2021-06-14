#include "ibedconf.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

#include <src/utils/confighandler.h>

IbedConf::IbedConf(QWidget* parent)
  : QWidget(parent)
{
    initWidgets();
    initLayout();
}
void IbedConf::updateComponents() {}
void IbedConf::initWidgets()
{
    m_username = new QLineEdit(this);
    m_username->setText(ConfigHandler().getUserName());
    // save
    m_saveButton = new QPushButton(tr("Save"), this);
    connect(m_saveButton, &QPushButton::clicked, this, &IbedConf::saveIBed);
    m_saveButton->setToolTip(tr("Saves the pattern"));
    // reset
    m_resetButton = new QPushButton(tr("Reset"), this);
    connect(m_resetButton, &QPushButton::clicked, this, &IbedConf::resetName);
    m_resetButton->setToolTip(tr("Restores the saved pattern"));
    // clear
    m_clearButton = new QPushButton(tr("Clear"), this);
    connect(m_clearButton, &QPushButton::clicked, this, [this]() {
        m_username->setText(QString());
    });
    m_clearButton->setToolTip(tr("Deletes the name"));
}
void IbedConf::initLayout()
{
    m_layout = new QVBoxLayout();
    m_layout->setAlignment(Qt::AlignTop);
    setLayout(m_layout);
    auto infoLabel = new QLabel(tr("username(cocall):"), this);
    infoLabel->setFixedHeight(20);
    QHBoxLayout* horizLayout0 = new QHBoxLayout();
    horizLayout0->addWidget(infoLabel);
    horizLayout0->addWidget(m_username);
    QHBoxLayout* horizLayout = new QHBoxLayout();
    horizLayout->addWidget(m_saveButton);
    horizLayout->addWidget(m_resetButton);
    horizLayout->addWidget(m_clearButton);
    m_layout->addLayout(horizLayout0);
    m_layout->addLayout(horizLayout);
}
void IbedConf::saveIBed()
{
    QString name = m_username->text();
    ConfigHandler().setUserName(name);
}

void IbedConf::resetName()
{
    QString name = "thunisoft";
    m_username->setText(ConfigHandler().getUserName());
}
