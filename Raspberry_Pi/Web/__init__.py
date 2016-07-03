from flask import Flask, render_template
from dbconnect import connect
app = Flask(__name__)

@app.route("/")
@app.route("/index")
def index():
  return render_template("index.html")
  
@app.route("/db_test", methods=["GET", "POST"])
def db_test():
    try:
        cursor, connection = connect()
        return("OK!")
    except Exception as e:
        return(str(e))
  
if __name__ == "__main__":
    app.run() 