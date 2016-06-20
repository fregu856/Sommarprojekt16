# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'designer_test.ui'
#
# Created by: PyQt4 UI code generator 4.11.4
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s

try:
    _encoding = QtGui.QApplication.UnicodeUTF8
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig, _encoding)
except AttributeError:
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig)

class Ui_mainWindow(object):
    def setupUi(self, mainWindow):
        mainWindow.setObjectName(_fromUtf8("mainWindow"))
        mainWindow.resize(800, 600)
        self.centralwidget = QtGui.QWidget(mainWindow)
        self.centralwidget.setObjectName(_fromUtf8("centralwidget"))
        self.gridLayout = QtGui.QGridLayout(self.centralwidget)
        self.gridLayout.setObjectName(_fromUtf8("gridLayout"))
        self.push_me = QtGui.QPushButton(self.centralwidget)
        self.push_me.setObjectName(_fromUtf8("push_me"))
        self.gridLayout.addWidget(self.push_me, 2, 0, 1, 1)
        self.pushButton = QtGui.QPushButton(self.centralwidget)
        self.pushButton.setObjectName(_fromUtf8("pushButton"))
        self.pushButton.clicked.connect(self.close_application) # EGEN RAD!
        self.gridLayout.addWidget(self.pushButton, 3, 1, 1, 1)
        self.or_me = QtGui.QPushButton(self.centralwidget)
        self.or_me.setObjectName(_fromUtf8("or_me"))
        self.gridLayout.addWidget(self.or_me, 2, 1, 1, 1)
        mainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtGui.QMenuBar(mainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 800, 26))
        self.menubar.setObjectName(_fromUtf8("menubar"))
        self.menuTest = QtGui.QMenu(self.menubar)
        self.menuTest.setObjectName(_fromUtf8("menuTest"))
        self.menuTest_2 = QtGui.QMenu(self.menubar)
        self.menuTest_2.setObjectName(_fromUtf8("menuTest_2"))
        mainWindow.setMenuBar(self.menubar)
        self.statusbar = QtGui.QStatusBar(mainWindow)
        self.statusbar.setObjectName(_fromUtf8("statusbar"))
        mainWindow.setStatusBar(self.statusbar)
        self.actionTest = QtGui.QAction(mainWindow)
        self.actionTest.setObjectName(_fromUtf8("actionTest"))
        self.actionTest_2 = QtGui.QAction(mainWindow)
        self.actionTest_2.setObjectName(_fromUtf8("actionTest_2"))
        self.actionTest_3 = QtGui.QAction(mainWindow)
        self.actionTest_3.setObjectName(_fromUtf8("actionTest_3"))
        self.menuTest.addAction(self.actionTest)
        self.menuTest.addAction(self.actionTest_2)
        self.menuTest_2.addAction(self.actionTest_3)
        self.menubar.addAction(self.menuTest.menuAction())
        self.menubar.addAction(self.menuTest_2.menuAction())

        self.retranslateUi(mainWindow)
        QtCore.QMetaObject.connectSlotsByName(mainWindow)

    def retranslateUi(self, mainWindow):
        mainWindow.setWindowTitle(_translate("mainWindow", "Test", None))
        self.push_me.setText(_translate("mainWindow", "Push me!", None))
        self.pushButton.setText(_translate("mainWindow", "PushButton", None))
        self.or_me.setText(_translate("mainWindow", "Or me", None))
        self.menuTest.setTitle(_translate("mainWindow", "Test", None))
        self.menuTest_2.setTitle(_translate("mainWindow", "Test 2", None))
        self.actionTest.setText(_translate("mainWindow", "Test", None))
        self.actionTest_2.setText(_translate("mainWindow", "Test 2", None))
        self.actionTest_3.setText(_translate("mainWindow", "Test", None))
    
    # Egen funktion!
    def close_application(self):
        
        choice = QtGui.QMessageBox.question(mainWindow, 'Warning', "Do you want to quit?", QtGui.QMessageBox.Yes | QtGui.QMessageBox.No)
        if choice == QtGui.QMessageBox.Yes:
            print("Applications is closed")
            sys.exit()
        else:
            pass


if __name__ == "__main__":
    import sys
    app = QtGui.QApplication(sys.argv)
    mainWindow = QtGui.QMainWindow()
    ui = Ui_mainWindow()
    ui.setupUi(mainWindow)
    mainWindow.show()
    sys.exit(app.exec_())

