/*
 * Copyright 2009 EMBITEL (http://www.embitel.com)
 * 
 * This file is part of Virtual Keyboard Project.
 * 
 * Virtual Keyboard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation
 * 
 * Virtual Keyboard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Virtual Keyboard. If not, see <http://www.gnu.org/licenses/>.
 */

#include "WidgetKeyboard.h"
#include <QKeyEvent>
#include <QSignalMapper>
#include <QSound>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QClipboard>
#include <QList>

WidgetKeyboard::WidgetKeyboard(QWidget *parent) : QWidget(0)
{
    setupUi(this);
    resize(0,0);
    //this->setWindowFlags(Qt::Tool);
    this->setWindowFlags(Qt::Dialog);
    m_pParent = parent;
    isCaps = false;
    isShift = false;
    isCtrl = false;
    isAlt = false;
	isIns = false;
    changeTextCaps(false);
    signalMapper = new QSignalMapper(this);
    sliderOpacity->setRange(20,100);
    allButtons = findChildren<QToolButton *>();
    for (int i=0;i<allButtons.count();i++) {
        connect(allButtons.at(i), SIGNAL(clicked()), signalMapper, SLOT(map()));
        signalMapper->setMapping(allButtons.at(i), i);
    }
    connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(on_btn_clicked(int)));
}

WidgetKeyboard::~WidgetKeyboard()
{
	delete signalMapper;
}

void WidgetKeyboard::on_btn_clicked(int btn)
{
	if(! soundFilePath.isEmpty())
		QSound::play(soundFilePath);
	QString strKeyId;
    strKeyId = allButtons.at(btn)->whatsThis();
    bool isOk;
    int keyId = strKeyId.toInt(&isOk, 16);
    if (strKeyId.isEmpty() || !isOk) {
        QMessageBox::information(0,0,"Key Not Found");
        return;
    }
    
    //Keys to be handled separately
	if (keyId==Qt::Key_Shift
		|| keyId==Qt::Key_Control
		|| keyId==Qt::Key_Alt
		|| keyId==Qt::Key_Print
		)
        return;

    int involvedKeys = 1;
    Qt::KeyboardModifiers Modifier = Qt::NoModifier;
    if (isCtrl) {
        Modifier = Modifier | Qt::ControlModifier;
        involvedKeys++;
    }
    if (isShift) {
        Modifier = Modifier | Qt::ShiftModifier;
        involvedKeys++;
    }
    if (isAlt) {
        Modifier = Modifier | Qt::AltModifier;
        involvedKeys++;
    }    
    
    bool isTextKey = false;

	if(keyId==Qt::Key_Insert && !isShift)
		return;
	

	QString ch = allButtons.at(btn)->text().trimmed();
    if (ch=="&&")
        ch="&";
	else if (keyId==Qt::Key_Space)
        ch = " ";
    else if (checkNotTextKey(keyId))
		ch = QString();
	else
		isTextKey = true;
	    

	if(isIns && isTextKey) {
		QKeyEvent keyEventIns(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
		QApplication::sendEvent(m_pParent->focusWidget(), &keyEventIns);
	}

	QKeyEvent keyEvent(QEvent::KeyPress, keyId, Modifier, ch, false, involvedKeys);
    QApplication::sendEvent(m_pParent->focusWidget(), &keyEvent);

    
    btnShiftLeft->setChecked(false);
	btnShiftRight->setChecked(false);
    btnCtrlLeft->setChecked(false);
    btnAltLeft->setChecked(false);
}

void WidgetKeyboard::on_btnCaps_toggled(bool checked)
{
    changeTextCaps(checked);
    isCaps = checked;
}

void WidgetKeyboard::on_btnShiftLeft_toggled(bool checked)
{
    isShift = checked;
    if (isCaps) {
        changeTextShift(checked);
    }
    else {
        changeTextShift(checked);
        changeTextCaps(checked);
    }
}

void WidgetKeyboard::on_btnShiftRight_toggled(bool checked)
{
    on_btnShiftLeft_toggled(checked);
}

void WidgetKeyboard::on_btnCtrlLeft_toggled(bool checked)
{
    isCtrl = checked;
}

void WidgetKeyboard::on_btnAltLeft_toggled(bool checked)
{
    isAlt = checked;
}

void WidgetKeyboard::on_btnIns_clicked()
{
	isIns = !isIns;
}

void WidgetKeyboard::on_btnPrint_clicked()
{
	QPixmap screen = QPixmap::grabWindow(QApplication::desktop()->winId());
	QClipboard *cb = QApplication::clipboard();
	cb->setPixmap(screen, QClipboard::Clipboard);
}

void WidgetKeyboard::on_sliderOpacity_valueChanged(int val)
{
    opacity = 120.0 - val;
    opacity /= 100.0;    
    setWindowOpacity(opacity);    
}

void WidgetKeyboard::changeTextShift(bool isShift)
{    
    changeTextCaps(!isShift);
    if (isShift) {
		btnTilt->setText(QChar('~'));
        btn1->setText(QChar('!'));
        btn2->setText(QChar('@'));
        btn3->setText(QChar('#'));
        btn4->setText(QChar('$'));
        btn5->setText(QChar('%'));
        btn6->setText(QChar('^'));        
        btn7->setText("&&");
        btn8->setText(QChar('*'));
        btn9->setText(QChar('('));
        btn0->setText(QChar(')'));
        btnHiphen->setText(QChar('_'));
        btnAssign->setText(QChar('+'));
        
        btnStartSquare->setText(QChar('{'));
        btnCloseSquare->setText(QChar('}'));
        btnFwdSlash->setText(QChar('|'));
        
        btnSemiColon->setText(QChar(':'));
        btnSp->setText(QChar('"'));
            
        btnComma->setText(QChar('<'));
        btnPeriod->setText(QChar('>'));
        btnBcwdSlash->setText(QChar('?'));
	}
    else {
		btnTilt->setText(QChar('`'));
        btn1->setText(QChar('1'));
        btn2->setText(QChar('2'));
        btn3->setText(QChar('3'));
        btn4->setText(QChar('4'));
        btn5->setText(QChar('5'));
        btn6->setText(QChar('6'));
        btn7->setText(QChar('7'));
        btn8->setText(QChar('8'));
        btn9->setText(QChar('9'));
        btn0->setText(QChar('0'));
        btnHiphen->setText(QChar('-'));
        btnAssign->setText(QChar('='));
            
        btnStartSquare->setText(QChar('['));
        btnCloseSquare->setText(QChar(']'));
        btnFwdSlash->setText(QChar('\\'));
        
        btnSemiColon->setText(QChar(';'));
        btnSp->setText(QChar('\''));
        
        btnComma->setText(QChar(','));
        btnPeriod->setText(QChar('.'));
        btnBcwdSlash->setText(QChar('/'));
    }
}

void WidgetKeyboard::changeTextCaps(bool isCaps)
{    
    if (isCaps) {
        btnQ->setText(QChar('Q'));
        btnW->setText(QChar('W'));
        btnE->setText(QChar('E'));
        btnR->setText(QChar('R'));
        btnT->setText(QChar('T'));
        btnY->setText(QChar('Y'));
        btnU->setText(QChar('U'));
        btnI->setText(QChar('I'));
        btnO->setText(QChar('O'));
        btnP->setText(QChar('P'));

        btnA->setText(QChar('A'));
        btnS->setText(QChar('S'));
        btnD->setText(QChar('D'));
        btnF->setText(QChar('F'));
        btnG->setText(QChar('G'));
        btnH->setText(QChar('H'));
        btnJ->setText(QChar('J'));
        btnK->setText(QChar('K'));
        btnL->setText(QChar('L'));
                
        btnZ->setText(QChar('Z'));
        btnX->setText(QChar('X'));
        btnC->setText(QChar('C'));
        btnV->setText(QChar('V'));
        btnB->setText(QChar('B'));
        btnN->setText(QChar('N'));
        btnM->setText(QChar('M'));        
    }
	else {
        btnQ->setText(QChar('q'));
        btnW->setText(QChar('w'));
        btnE->setText(QChar('e'));
        btnR->setText(QChar('r'));
        btnT->setText(QChar('t'));
        btnY->setText(QChar('y'));
        btnU->setText(QChar('u'));
        btnI->setText(QChar('i'));
        btnO->setText(QChar('o'));
        btnP->setText(QChar('p'));

        btnA->setText(QChar('a'));
        btnS->setText(QChar('s'));
        btnD->setText(QChar('d'));
        btnF->setText(QChar('f'));
        btnG->setText(QChar('g'));
        btnH->setText(QChar('h'));
        btnJ->setText(QChar('j'));
        btnK->setText(QChar('k'));
        btnL->setText(QChar('l'));
        
        btnZ->setText(QChar('z'));
        btnX->setText(QChar('x'));
        btnC->setText(QChar('c'));
        btnV->setText(QChar('v'));
        btnB->setText(QChar('b'));
        btnN->setText(QChar('n'));
        btnM->setText(QChar('m'));
    }
}

bool WidgetKeyboard::checkNotTextKey(int keyId)
{
	if (keyId==Qt::Key_Shift
            || keyId==Qt::Key_Control
            || keyId==Qt::Key_Tab
            || keyId==Qt::Key_Escape
            || keyId==Qt::Key_Return
            || keyId==Qt::Key_Insert
            || keyId==Qt::Key_NumLock
            || keyId==Qt::Key_F1
            || keyId==Qt::Key_F2
            || keyId==Qt::Key_F3
            || keyId==Qt::Key_F4
            || keyId==Qt::Key_F5
            || keyId==Qt::Key_F6
            || keyId==Qt::Key_F7
            || keyId==Qt::Key_F8
            || keyId==Qt::Key_F9
            || keyId==Qt::Key_F10
            || keyId==Qt::Key_F11
            || keyId==Qt::Key_F12
            || keyId==Qt::Key_Print
            || keyId==Qt::Key_Pause
            || keyId==Qt::Key_ScrollLock
            || keyId==Qt::Key_Enter
            || keyId==Qt::Key_Home
            || keyId==Qt::Key_End
            || keyId==Qt::Key_CapsLock
            || keyId==Qt::Key_Insert
            || keyId==Qt::Key_Delete
            || keyId==Qt::Key_PageUp
            || keyId==Qt::Key_PageDown
            || keyId==Qt::Key_Down
            || keyId==Qt::Key_Up
            || keyId==Qt::Key_Left
            || keyId==Qt::Key_Right
			|| keyId==Qt::Key_Alt) {
				return true;
	}
	else
		return false;
}

void WidgetKeyboard::setOpacity(int opacity)
{
    sliderOpacity->setValue(opacity);
}

int WidgetKeyboard::getOpacity()
{
    return sliderOpacity->value();
}

void WidgetKeyboard::setSoundDisabled()
{
	soundFilePath = "";
}

/* 
Microsoft Windows : The underlying multimedia system is used; only WAVE format sound files are supported.
X11	: The Network Audio System is used if available, otherwise all operations work silently. NAS supports WAVE and AU files.
Mac OS X : NSSound is used. All formats that NSSound supports, including QuickTime formats, are supported by Qt for Mac OS X.
Qt for Embedded Linux : A built-in mixing sound server is used, accessing /dev/dsp directly. Only the WAVE format is supported.
*/

void WidgetKeyboard::setSoundEnabled(QString soundPath)
{
	soundFilePath = soundPath;
}

void WidgetKeyboard::on_checkBoxShowNumPad_stateChanged(int state)
{
	QList<QWidget *> children = frameNumPad->findChildren<QWidget *>();
	if(state==Qt::Checked)
	{
		frameNumPad->show();
		/*for each(QWidget *child in children)
			child->show();*/
	}
	else
	{
		frameNumPad->hide();
		/*for each(QWidget *child in children)
			child->hide();*/
	}
	update();
}
