import sys
from PyQt4 import QtCore, QtGui

class Window(QtGui.QMainWindow):	# Klassen "Window" ärver från Qt.Gui.QMainWindow
	
	def __init__(self):	# Metod som alltid kommer köras direkt när en instans av "Window" skapas (här läggs det som alltid ska köras direkt vid uppstart)
		
		super(Window, self).__init__()	# super: ger föräldern. Dvs, kör init-metoden för klassen (QMainWindow) vi ärver ifrån
		
		self.setGeometry(50, 50, 500, 300)	# self: det egna objektet, instansen av klassen som skapats
		self.setWindowTitle("Test-namn")
		self.setWindowIcon(QtGui.QIcon('logo.png'))
		
		statusBar = self.statusBar()	# Skapa/visa status-bar:en (bar:en längst ner på fönstret)
		
		extractAction = QtGui.QAction("&GET TO THE CHOPPAH!", self)
		extractAction.setShortcut("Ctrl+Q")
		extractAction.setStatusTip('Leave the App')	# Ange vad status-bar:en ska visa 
		extractAction.triggered.connect(self.close_application)	# Ange vad som ska hända när den triggas
		
		mainMenu = self.menuBar()	# Skapa meny-bar:en
		fileMenu = mainMenu.addMenu('&File')	# Lägg till "File"-dropdown
		fileMenu.addAction(extractAction)	# Lägg till rad i "File"-dropdown:en
				
		self.home()	# home(): egendef. funktion, se nedan
		
	def home(self):
		
		button = QtGui.QPushButton("Quit", self)	# Skapa en knapp (med texten "Quit")
		button.clicked.connect(self.close_application)	# Ange vad som ska hända när knappen klickas på (stänga ner programmet i det här fallet)
		button.resize(100, 100)	# Ge knappen storlek 100x100
		button.move(100, 100)	# Flytta knappen
		
		extractAction2 = QtGui.QAction(QtGui.QIcon('logo.png'), 'Testelitest', self)
		extractAction2.triggered.connect(self.close_application)
		
		self.toolBar = self.addToolBar("Extraction")
		self.toolBar.addAction(extractAction2)
		
		self.show()
		
	def close_application(self):
		
		choice = QtGui.QMessageBox.question(self, 'Warning', "Do you want to quit?", QtGui.QMessageBox.Yes | QtGui.QMessageBox.No)
		if choice == QtGui.QMessageBox.Yes:
			print("Applications is closed")
			sys.exit()
		else:
			pass
		
def run():		
	
	app = QtGui.QApplication(sys.argv)
	GUI = Window()
	sys.exit(app.exec_())	# 1: Kör app.excec_() (kör app, visa GUI:t osv). 2: (när app_exec_() är klar) kör sys.exit() med app.exec_():s returvärde som inargument, då stängs python-programmet ner och returvärdet skickas till operativsystemet (för att möjliggöra felsök om app avslutades oväntat)

run()
