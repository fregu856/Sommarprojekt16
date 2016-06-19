import sys
from PyQt4 import QtCore, QtGui

class Window(QtGui.QMainWindow):	# Klassen "Window" ärver från Qt.Gui.QMainWindow
	
	def __init__(self):	# Metod som alltid kommer köras direkt när en instans av "Window" skapas (här läggs det som alltid ska köras direkt vid uppstart)
		super(Window, self).__init__()	# super: ger föräldern. Dvs, kör init-metoden för klassen (QMainWindow) vi ärver ifrån
		self.setGeometry(50, 50, 500, 300)	# self: det egna objektet, instansen av klassen som skapats
		self.setWindowTitle("Test-namn")
		self.setWindowIcon(QtGui.QIcon('logo.png'))
		self.home()	# home(): egendef. funktion, se nedan
		
	def home(self):
		button = QtGui.QPushButton("Quit", self)	# Skapa en knapp (med texten "Quit")
		button.clicked.connect(self.close_application)	# Ange vad som ska hända när knappen klickas på (stänga ner programmet i det här fallet)
		button.resize(100, 100)	# Ge knappen storlek 100x100
		button.move(100, 100)	# Flytta knappen
		self.show()
		
	def close_application(self):
		print("Application has closed")
		sys.exit()
		
def run():		
	app = QtGui.QApplication(sys.argv)
	GUI = Window()
	sys.exit(app.exec_())	# 1: Kör app.excec_() (kör app, visa GUI:t osv). 2: (när app_exec_() är klar) kör sys.exit() med app.exec_():s returvärde som inargument, då stängs python-programmet ner och returvärdet skickas till operativsystemet (för att möjliggöra felsök om app avslutades oväntat)

run()
