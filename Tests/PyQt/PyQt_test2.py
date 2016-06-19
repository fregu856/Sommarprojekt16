import sys
from PyQt4 import QtCore, QtGui

class Window(QtGui.QMainWindow):	# Klassen "Window" ärver från Qt.Gui.QMainWindow
	
	def __init__(self):	# Metod som alltid kommer köras när en instans av "Window" skapas
		super(Window, self).__init__()	# super: ger föräldern. Dvs, kör init-metoden för klassen (QMainWindow) vi ärver ifrån
		self.setGeometry(50, 50, 500, 300)	# self: det egna objektet, instansen av klassen som skapats
		self.setWindowTitle("Test-namn")
		self.setWindowIcon(QtGui.QIcon('logo.png'))
		self.show()
		
app = QtGui.QApplication(sys.argv)
GUI = Window()
sys.exit(app.exec_())	# 1: Kör app.excec_() (kör app, visa GUI:t osv). 2: (när app_exec_() är klar) kör sys.exit() med app.exec_():s returvärde som inargument, då stängs python-programmet ner och returvärdet skickas till operativsystemet (för att möjliggöra felsök om app avslutades oväntat)

