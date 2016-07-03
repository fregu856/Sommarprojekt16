import MySQLdb

def connect():
    connection = MySQLdb.connect(host="localhost", user="pi", passwd="raspberry", db="SQL_DATA")
    cursor = connection.cursor()
    
    return cursor, connection