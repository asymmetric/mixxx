/**
 * @file dlgprefnewsounditem.cpp
 * @author Bill Good <bkgood at gmail dot com>
 * @date 20100704
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dlgprefnewsounditem.h"
#include "sounddevice.h"
#include "soundmanagerconfig.h"

/**
 * Constructs a new preferences sound item, representing an AudioPath and SoundDevice
 * with a label and two combo boxes.
 * @param type The AudioPathType of the path to be represented
 * @param devices The list of devices for the user to choose from (either a collection
 * of input or output devices).
 * @param isInput true if this is representing an AudioInput, false otherwise
 * @param index the index of the represented AudioPath, if applicable
 */
DlgPrefNewSoundItem::DlgPrefNewSoundItem(QWidget *parent, AudioPathType type,
        QList<SoundDevice*> &devices, bool isInput,
        unsigned int index /* = 0 */)
    : QWidget(parent)
    , m_type(type)
    , m_index(index)
    , m_devices(devices)
    , m_isInput(isInput) {
    setupUi(this);
    if (AudioPath::isIndexed(type)) {
        typeLabel->setText(
            QString("%1 %2").arg(AudioPath::getStringFromType(type)).arg(index + 1));
    } else {
        typeLabel->setText(AudioPath::getStringFromType(type));
    }
    deviceComboBox->addItem("None" /* translate me */, "None" /*don't translate me */);
    connect(deviceComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(deviceChanged(int)));
    connect(channelComboBox, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(settingChanged()));
    refreshDevices(m_devices);
}

DlgPrefNewSoundItem::~DlgPrefNewSoundItem() {

}

/**
 * Slot called when the parent preferences pane updates its list of sound
 * devices, to update the item widget's list of devices to display.
 */
void DlgPrefNewSoundItem::refreshDevices(const QList<SoundDevice*> &devices) {
    m_devices = devices;
    deviceComboBox->setCurrentIndex(0);
    // not using combobox->clear means we can leave in "None" so it
    // doesn't flicker when you switch APIs... cleaner Mixxx :) bkgood
    while (deviceComboBox->count() > 1) {
        deviceComboBox->removeItem(deviceComboBox->count() - 1);
    }
    foreach (SoundDevice *device, m_devices) {
        deviceComboBox->addItem(device->getDisplayName(), device->getInternalName());
    }
}

/**
 * Slot called when the device combo box selection changes. Updates the channel
 * combo box.
 */
void DlgPrefNewSoundItem::deviceChanged(int index) {
    // TODO(bkgood) assumes we're looking for a two-channel device -- eventually
    // will want to give the option of mono for a microphone, depending on the
    // value of m_type (this will be an easy fix when there's time)
    channelComboBox->clear();
    QString selection = deviceComboBox->itemData(index).toString();
    unsigned int numChannels = 0;
    if (selection == "None") {
        goto emitAndReturn;
    } else {
        foreach (SoundDevice *device, m_devices) {
            if (device->getInternalName() == selection) {
                if (m_isInput) {
                    numChannels = device->getNumInputChannels();
                } else {
                    numChannels = device->getNumOutputChannels();
                }
            }
        }
    }
    if (numChannels == 0) {
        goto emitAndReturn;
    } else {
        for (unsigned int i = 1; i + 1 <= numChannels; ++i) {
            channelComboBox->addItem(
                QString("Channels %1 - %2").arg(i).arg(i + 1), i - 1);
            // i-1 because want the data part to be what goes into audiopath's
            // channelbase which is zero-based -- bkgood
        }
    }
emitAndReturn:
    emit(settingChanged());
}

void DlgPrefNewSoundItem::loadPath(const SoundManagerConfig &config) {
    if (m_isInput) {
        QMultiHash<SoundDevice*, AudioInput> inputs(config.getInputs());
        foreach (SoundDevice *dev, inputs.uniqueKeys()) {
            foreach (AudioInput in, inputs.values(dev)) {
                if (in.getType() == m_type && in.getIndex() == m_index) {
                    setDevice(dev);
                    setChannel(in.getChannelGroup().getChannelBase());
                    return; // we're just using the first one found, leave
                            // multiples to a more advanced dialog -- bkgood
                }
            }
        }
    } else {
        QMultiHash<SoundDevice*, AudioOutput> outputs(config.getOutputs());
        foreach (SoundDevice *dev, outputs.uniqueKeys()) {
            foreach (AudioOutput out, outputs.values(dev)) {
                if (out.getType() == m_type && out.getIndex() == m_index) {
                    setDevice(dev);
                    setChannel(out.getChannelGroup().getChannelBase());
                    return; // we're just using the first one found, leave
                            // multiples to a more advanced dialog -- bkgood
                }
            }
        }
    }
}

/**
 * Slot called when the underlying DlgPrefNewSound wants this Item to
 * record its respective path with the SoundManagerConfig instance at
 * config.
 */
void DlgPrefNewSoundItem::writePath(SoundManagerConfig *config) const {
    SoundDevice *device = getDevice();
    if (device == NULL) {
        return;
    } // otherwise, this will have a valid audiopath
    if (m_isInput) {
        config->addInput(
                device,
                AudioInput(
                    m_type,
                    channelComboBox->itemData(channelComboBox->currentIndex()).toUInt(),
                    m_index
                    )
                );
    } else {
        config->addOutput(
                device,
                AudioOutput(
                    m_type,
                    channelComboBox->itemData(channelComboBox->currentIndex()).toUInt(),
                    m_index
                    )
                );
    }
}

/**
 * Gets the currently selected SoundDevice
 * @returns pointer to SoundDevice, or NULL if the "None" option is selected.
 */
SoundDevice* DlgPrefNewSoundItem::getDevice() const {
    QString selection = deviceComboBox->itemData(deviceComboBox->currentIndex()).toString();
    if (selection == "None") {
        return NULL;
    }
    foreach (SoundDevice *device, m_devices) {
        if (selection == device->getInternalName()) {
            return device;
        }
    }
    // looks like something became invalid ???
    deviceComboBox->setCurrentIndex(0); // set it to none
    return NULL;
}

void DlgPrefNewSoundItem::setDevice(const SoundDevice *device) {
    int index = deviceComboBox->findData(device->getInternalName());
    if (index != -1) {
        deviceComboBox->setCurrentIndex(index);
    } else {
        deviceComboBox->setCurrentIndex(0); // None
    }
}

void DlgPrefNewSoundItem::setChannel(unsigned int channel) {
    int index = channelComboBox->findData(channel);
    if (index != -1) {
        channelComboBox->setCurrentIndex(index);
    } else {
        channelComboBox->setCurrentIndex(0); // 1
    }
}
