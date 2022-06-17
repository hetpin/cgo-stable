/********************************************************************************
** Form generated from reading UI file 'dialogimpulsenoise.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGIMPULSENOISE_H
#define UI_DIALOGIMPULSENOISE_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>

QT_BEGIN_NAMESPACE

class Ui_DialogImpulseNoise
{
public:
    QDialogButtonBox *buttonBox;
    QGroupBox *groupBox;
    QLabel *label;
    QSpinBox *spinBox;

    void setupUi(QDialog *DialogImpulseNoise)
    {
        if (DialogImpulseNoise->objectName().isEmpty())
            DialogImpulseNoise->setObjectName(QStringLiteral("DialogImpulseNoise"));
        DialogImpulseNoise->resize(201, 120);
        buttonBox = new QDialogButtonBox(DialogImpulseNoise);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setGeometry(QRect(20, 80, 171, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        groupBox = new QGroupBox(DialogImpulseNoise);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(20, 0, 161, 81));
        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(20, 30, 56, 13));
        spinBox = new QSpinBox(groupBox);
        spinBox->setObjectName(QStringLiteral("spinBox"));
        spinBox->setGeometry(QRect(100, 20, 53, 25));
        spinBox->setValue(15);

        retranslateUi(DialogImpulseNoise);
        QObject::connect(buttonBox, SIGNAL(accepted()), DialogImpulseNoise, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), DialogImpulseNoise, SLOT(reject()));

        QMetaObject::connectSlotsByName(DialogImpulseNoise);
    } // setupUi

    void retranslateUi(QDialog *DialogImpulseNoise)
    {
        DialogImpulseNoise->setWindowTitle(QApplication::translate("DialogImpulseNoise", "Dialog", 0));
        groupBox->setTitle(QApplication::translate("DialogImpulseNoise", "Impulse noise parameters", 0));
        label->setText(QApplication::translate("DialogImpulseNoise", "Rate:", 0));
    } // retranslateUi

};

namespace Ui {
    class DialogImpulseNoise: public Ui_DialogImpulseNoise {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGIMPULSENOISE_H
