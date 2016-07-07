from flask import Flask, render_template, request, redirect, url_for, session, flash
from dbconnect import connect
import gc   # Garbage collection
from datetime import date
import numbers

app = Flask(__name__)

@app.route("/")   
@app.route("/index")
def index():
    try:
        return render_template("index.html") 
    except Exception as e:
        return render_template("500.html", error = str(e))
  
@app.errorhandler(404)
def page_not_found(e):
    try:
        return render_template("404.html") 
    except Exception as e:
        return render_template("500.html", error = str(e))
  
if __name__ == "__main__":
    app.run() 