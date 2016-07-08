import csv                          # CSV
import xml.etree.ElementTree as ET  # XML
from dbconnect import connect       # For att ansluta till databasen
import os                           # (For att kunna lista och ta bort filer)
from os.path import isfile, join    # (For att kunna lista och ta bort filer)
import shutil                       # For att kunna flytta filer
from math import floor

##################################################################
# INITIERING AV VARIABLER & PARAMETRAR
##################################################################
duration = 0
distance = 0
VIN = ""
logger = ""
OBD_DTC_number = 0
UDS_DTC_number = 0
UDS_SECOND_TESTER_DETECTED = 0 
run_date = ""
run_time = ""
network_name = ""
network_address = ""
GPS_lat_start = 0
GPS_long_start = 0
GPS_lat_stop = 0
GPS_long_stop = 0
country = ""
signals_name = []
signals_unit = []
signals_info = []
OBD_DTC_list = []
OBD_DTC_list_text = []
OBD_PENDING_DTC_list = []
OBD_PENDING_DTC_list_text = []
UDS_DTC_list = []
UDS_DTC_list_text = []
UDS_PENDING_DTC_list = []
UDS_PENDING_DTC_list_text = []
data = {}
timestamps = {}
dir_path = "/home/pi/Test/Testdata"   # Mappen dar det ar tankt att den nya datan fran sftp-servern ligger

##################################################################
# FUNKTIONER
##################################################################
def csv_get_info(csv_reader, csv_file, info_name):
    for row in csv_reader:
        if row[0] == info_name: 
            info_value = row[1]
            csv_file.seek(0)    # aterpositionera till borjan av filen
            return info_value       
                        
def csv_get_DTC_info(csv_reader, csv_file):
    global OBD_DTC_number, OBD_DTC_list, OBD_DTC_list_text
    global UDS_DTC_number, UDS_DTC_list, UDS_DTC_list_text
    global OBD_PENDING_DTC_list, OBD_PENDING_DTC_list_text
    global UDS_PENDING_DTC_list, UDS_PENDING_DTC_list_text
    
    OBD_DTC_number = int(csv_get_info(csv_reader, csv_file, "OBD_DTC_NUMBER"))
    UDS_DTC_number = int(csv_get_info(csv_reader, csv_file, "UDS_DTC_NUMBER"))
    for row in csv_reader:
        if row[0] == "OBD_DTC_LIST" and len(row) > 1:
            for index in range(1, len(row)):
                if row[index]:  # Om inte tomt element:
                    if index == 1:
                        sender_address, DTC_ID = row[index].split(":")  # Forsta felkoden anges pa format enligt ex "7E8:P1A0F00", ovriga bara "P1A0F00"
                        OBD_DTC_list.append(DTC_ID)
                    else:   
                        OBD_DTC_list.append(row[index])
        
        elif row[0] == "OBD_PENDING_DTC_LIST" and len(row) > 1:
            for index in range(1, len(row)):
                if row[index]:  # Om inte tomt element: 
                    if index == 1:
                        sender_address, DTC_ID = row[index].split(":")  # Forsta felkoden anges pa format enligt ex "7E8:P1A0F00", ovriga bara "P1A0F00"
                        OBD_PENDING_DTC_list.append(DTC_ID)
                    else:   
                        OBD_PENDING_DTC_LIST.append(row[index])
        
        elif row[0] == "UDS_DTC_LIST" and len(row) > 1:
            for index in range(1, len(row)):
                if row[index]:  # Om inte tomt element:
                    if index == 1:
                        sender_address, DTC_ID = row[index].split(":")
                        UDS_DTC_list.append(DTC_ID)
                    else:   
                        UDS_DTC_list.append(row[index])
                    
        elif row[0] == "UDS_PENDING_DTC_LIST" and len(row) > 1:
            for index in range(1, len(row)):
                if row[index]:  # Om inte tomt element
                    if index == 1:
                        sender_address, DTC_ID = row[index].split(":")
                        UDS_PENDING_DTC_list.append(DTC_ID)
                    else:   
                        UDS_PENDING_DTC_list.append(row[index])            
        
        elif row[0] == "OBD_DTC_LIST_TEXT" and len(row) > 1:
            for index in range(1, len(row)):
                if row[index]:  # Om inte tomt element
                    OBD_DTC_list_text.append(row[index])
             
        elif row[0] == "OBD_PENDING_DTC_LIST_TEXT" and len(row) > 1:
            for index in range(1, len(row)):
                if row[index]:  # Om inte tomt element
                    OBD_PENDING_DTC_list_text.append(row[index])     
        
        elif row[0] == "UDS_DTC_LIST_TEXT" and len(row) > 1:
            for index in range(1, len(row)):
                if row[index]:  # Om inte tomt element
                    UDS_DTC_list_text.append(row[index]) 
        
        elif row[0] == "UDS_PENDING_DTC_LIST_TEXT" and len(row) > 1:
            for index in range(1, len(row)):
                if row[index]:  # Om inte tomt element
                    UDS_PENDING_DTC_list_text.append(row[index])        
    
    csv_file.seek(0)    # aterpositionera till borjan av filen 

def csv_get_GPS_info(csv_reader, csv_file):
    global GPS_lat_start, GPS_long_start
    global GPS_lat_stop, GPS_long_stop
    
    for row in csv_reader:
        if row[0] == "GPS_START_POS" and len(row) > 1:
            GPS_long_start = float(row[1])
            GPS_lat_start = float(row[2])
        
        elif row[0] == "GPS_STOP_POS" and len(row) > 1:
            GPS_long_stop = float(row[1])
            GPS_lat_stop = float(row[2])        
    
    csv_file.seek(0)    # aterpositionera till borjan av filen 
    
            
def read_from_csv(file_name):
    global duration, distance, VIN, logger, OBD_DTC_list, OBD_DTC_list_text
    global OBD_DTC_number, UDS_DTC_list, UDS_DTC_list_text, UDS_DTC_number
    global signals_name, signals_unit, signals_info, data, timestamps
    global UDS_SECOND_TESTER_DETECTED
            
    csv_file = open(join(dir_path, file_name + ".csv"))
    csv_reader = csv.reader(csv_file)

    duration_sec = int(csv_get_info(csv_reader, csv_file, "JOURNEY_TIME"))
    duration = floor(duration_sec/60)
    distance = int(csv_get_info(csv_reader, csv_file, "ODO_JOURNEY"))
    VIN = csv_get_info(csv_reader, csv_file, "VIN")
    logger = csv_get_info(csv_reader, csv_file, "IMEI")
    
    UDS_TESTER_raw = csv_get_info(csv_reader, csv_file, "UDS_SECOND_TESTER_DETECTED")
    if UDS_TESTER_raw == "True":
        UDS_SECOND_TESTER_DETECTED = 1
    else:
        UDS_SECOND_TESTER_DETECTED = 0
         
    data_start_row = int(csv_get_info(csv_reader, csv_file, "Data_Section_StartRow"))
    
    csv_get_DTC_info(csv_reader, csv_file)
    csv_get_GPS_info(csv_reader, csv_file)

    counter = 1
    for row in csv_reader:
        if counter < data_start_row:
            counter +=1
    
        elif counter == data_start_row:
            signal_row = row
            for index in range(1, len(row), 2): # Bara intresserade av udda index, ty dar ligger signalerna (jamna: "ABS TIME")
                signals_name.append(row[index])
            for signal_name in signals_name:
                timestamps[signal_name] = []
                data[signal_name] = []  
            counter += 1

        elif counter == data_start_row + 1: # Bara intresserade av udda index, ty de hor till signalerna (jamna: "ABS TIME")
            for index in range(1, len(row), 2):
                signals_unit.append(row[index])
            counter += 1

        elif counter == data_start_row + 2: # Bara intresserade av udda index, ty de hor till signalerna (jamna: "ABS TIME")
            for index in range(1, len(row), 2):
                signals_info.append(row[index])
            counter += 1
        
        else:
            for index in range(1, len(row), 2):
                timestamps[signal_row[index]].append(row[index - 1])
                data[signal_row[index]].append(row[index])
            counter += 1    
            
    data, timestamps = remove_empty_data(data, timestamps)
    
    csv_file.seek(0)    # aterpositionera till borjan av filen
    csv_file.close()
    
def remove_empty_data(data_dict, timestamps_dict):
    for entry in timestamps_dict:
        entry_timestamps = timestamps_dict[entry]   # (entry_timestamps ar en vektor)
        entry_data = data_dict[entry]   # (entry_data ar en vektor)
        vector_size = len(entry_timestamps)
        first_empty_entry_index = -1
        for index in range(len(entry_timestamps)):
            if not entry_timestamps[index]: # Om vi har natt borjan pa den tomma datan:
                first_empty_entry_index = index
                break
        if(first_empty_entry_index >= 0):   # Ska ta bort och uppdatera endast om det fanns tom data att ta bort (om first_empty_entry_index andrades)      
            del entry_timestamps[first_empty_entry_index:vector_size]
            del entry_data[first_empty_entry_index:vector_size] 
            timestamps_dict[entry] = entry_timestamps
            data_dict[entry] = entry_data
    return data_dict, timestamps_dict
        
def convert_date_for_mySQL(date_from_odos):
    day, month, year = date_from_odos.split("-")
    date = year + "-" + month + "-" + day
    return date

def read_from_xml(file_name):   
    global run_date, run_time, network_name, network_address
    
    xml_tree = ET.parse(join(dir_path, file_name + ".xml"))
    root = xml_tree.getroot()

    run_date = convert_date_for_mySQL(root.get("date")) 
    run_time = root.get("time") 

    network_name = root.find("./vehicleinformation//network").text
    network_address = root.find("./vehicleinformation//address").text

def create_new_db():
    cursor, connection = connect()

    # Ta bort tabellerna sedan tidigare (s.a skriptet kan koras om i sin helhet)
    cursor.execute("DROP TABLE IF EXISTS signals")
    cursor.execute("DROP TABLE IF EXISTS OBD_DTCs")
    cursor.execute("DROP TABLE IF EXISTS UDS_DTCs")
    cursor.execute("DROP TABLE IF EXISTS test_runs")    # OBS! Maste ligga sist, ty tabellerna ovan har foreign key som pekar till denna 
    # Skapa tabellen for testkorningar
    sql = """CREATE TABLE test_runs(    
        id VARCHAR(100) NOT NULL,
        duration INT,
        distance INT,
        VIN VARCHAR(100) NOT NULL,
        logger VARCHAR(100) NOT NULL,
        OBD_DTC_number INT,
        UDS_DTC_number INT,
        UDS_SECOND_TESTER_DETECTED BOOLEAN,
        date DATE,
        time TIME,
        network_name VARCHAR(100) NOT NULL,
        network_address VARCHAR(100) NOT NULL,
        GPS_lat_start FLOAT,
        GPS_long_start FLOAT,
        GPS_lat_stop FLOAT,
        GPS_long_stop FLOAT,
        PRIMARY KEY (id)
        )"""
    cursor.execute(sql)

    # Skapa tabellen for signaler
    sql = """CREATE TABLE signals(  
        name VARCHAR(500) NOT NULL,
        unit VARCHAR(500),
        info VARCHAR(500),
        test_run_id VARCHAR(100) NOT NULL,
        PRIMARY KEY (name, info, test_run_id),
        FOREIGN KEY (test_run_id) REFERENCES test_runs(id)
        )"""
    cursor.execute(sql)

    # Skapa tabellen for  OBD-felkoder
    sql = """CREATE TABLE OBD_DTCs( 
        DTC_id VARCHAR(100) NOT NULL,
        DTC_text VARCHAR(100),
        pending BOOLEAN,
        test_run_id VARCHAR(100) NOT NULL,
        PRIMARY KEY (DTC_id, pending, test_run_id),
        FOREIGN KEY (test_run_id) REFERENCES test_runs(id)
        )"""
    cursor.execute(sql)

    # Skapa tabellen for  UDS-felkoder
    sql = """CREATE TABLE UDS_DTCs( 
        DTC_id VARCHAR(100) NOT NULL,
        DTC_text VARCHAR(100),
        pending BOOLEAN,
        test_run_id VARCHAR(100) NOT NULL,
        PRIMARY KEY (DTC_id, pending, test_run_id),
        FOREIGN KEY (test_run_id) REFERENCES test_runs(id)
        )"""
    cursor.execute(sql)
    
    connection.close()

def insert_into_sql(file_name):
    cursor, connection = connect()
    
    # Satt in i test_runs-tabellen (OBS! Ska vara "%s" aven pa siffror, MySQL funkar sa)
    query = """INSERT INTO test_runs VALUES(
        %s, %s, %s, %s, %s, %s, %s, %s, 
        %s, %s, %s, %s, %s, %s, %s, %s)"""
    query_data = (file_name, duration, distance, VIN, logger, OBD_DTC_number, UDS_DTC_number, UDS_SECOND_TESTER_DETECTED, run_date, run_time, network_name, network_address, GPS_lat_start, GPS_long_start, GPS_lat_stop, GPS_long_stop)
    cursor.execute(query, query_data)
    connection.commit()

    # Satt in i signals-tabellen
    query_data = []
    for index in range(len(signals_name)):
        query_subdata = []
        query_subdata.append(signals_name[index])
        query_subdata.append(signals_unit[index])
        query_subdata.append(signals_info[index])
        query_subdata.append(file_name)
        query_data.append(query_subdata)

    cursor.executemany("INSERT INTO signals VALUES(%s, %s, %s, %s)", query_data)
    connection.commit()

    # Satt in confirmed i OBD-tabellen 
    query_data = []
    for index in range(len(OBD_DTC_list)):
        query_subdata = []
        query_subdata.append(OBD_DTC_list[index])
        query_subdata.append(OBD_DTC_list_text[index])
        query_subdata.append(0) # Dessa ar INTE pending
        query_subdata.append(file_name)
        query_data.append(query_subdata)
         
    cursor.executemany("INSERT INTO OBD_DTCs VALUES(%s, %s, %s, %s)", query_data)
    connection.commit()
    
    # Satt in pending i OBD-tabellen 
    query_data = []
    for index in range(len(OBD_PENDING_DTC_list)):
        query_subdata = []
        query_subdata.append(OBD_PENDING_DTC_list[index])
        query_subdata.append(OBD_PENDING_DTC_list_text[index])
        query_subdata.append(1) # Dessa ar pending
        query_subdata.append(file_name)
        query_data.append(query_subdata)
         
    cursor.executemany("INSERT INTO OBD_DTCs VALUES(%s, %s, %s, %s)", query_data)
    connection.commit()

    # Satt in confirmed i UDS-tabellen
    query_data = []
    for index in range(len(UDS_DTC_list)):
        query_subdata = []
        query_subdata.append(UDS_DTC_list[index])
        query_subdata.append(UDS_DTC_list_text[index])
        query_subdata.append(0) # Dessa ar INTE pending
        query_subdata.append(file_name)
        query_data.append(query_subdata)
    
    cursor.executemany("INSERT INTO UDS_DTCs VALUES(%s, %s, %s, %s)", query_data)
    connection.commit() 
    
    # Satt in pending i UDS-tabellen
    query_data = []
    for index in range(len(UDS_DTC_list)):
        query_subdata = []
        query_subdata.append(UDS_DTC_list[index])
        query_subdata.append(UDS_DTC_list_text[index])
        query_subdata.append(1) # Dessa ar pending
        query_subdata.append(file_name)
        query_data.append(query_subdata)    
         
    cursor.executemany("INSERT INTO UDS_DTCs VALUES(%s, %s, %s, %s)", query_data)
    connection.commit() 
    
    connection.close()

def reset_variables():
    global duration, distance, VIN, logger, OBD_DTC_list, OBD_DTC_list_text
    global OBD_DTC_number, UDS_DTC_list, UDS_DTC_list_text, UDS_DTC_number
    global signals_name, signals_unit, signals_info
    global run_date, run_time, network_name 
    global network_address, data, timestamps
    global OBD_PENDING_DTC_list, OBD_PENDING_DTC_list_text
    global UDS_PENDING_DTC_list, UDS_PENDING_DTC_list_text
    global GPS_lat_start, GPS_long_start
    global GPS_lat_stop, GPS_long_stop, country
    
    duration = 0 
    distance = 0 
    VIN = ""
    logger = ""
    OBD_DTC_number = 0
    UDS_DTC_number = 0
    UDS_SECOND_TESTER_DETECTED = 0 
    run_date = ""
    run_time = ""
    network_name = ""
    network_address = ""
    GPS_lat_start = 0
    GPS_long_start = 0
    GPS_lat_stop = 0
    GPS_long_stop = 0
    country = ""
    signals_name = []
    signals_unit = []
    signals_info = []
    OBD_DTC_list = []
    OBD_DTC_list_text = []
    OBD_PENDING_DTC_list = []
    OBD_PENDING_DTC_list_text = []
    UDS_DTC_list = []
    UDS_DTC_list_text = []
    UDS_PENDING_DTC_list = []
    UDS_PENDING_DTC_list_text = []
    data = {}
    timestamps = {}
    
def add_to_db(file_name):
    reset_variables() 
    try:
        read_from_csv(file_name)
    except Exception as e:
        print("Could not read: " + file_name + ".csv")
        return  # Om inlasningen inte funkar avbryts hela forsoket och filerna far ligga kvar        
    try:
        read_from_xml(file_name)
    except Exception as e:
        print("Could not read: " + file_name + ".xml")
        return  # Om inlasningen inte funkar avbryts hela forsoket och filerna far ligga kvar
    insert_into_sql(file_name)
    
    # Skriv in filnamnet i listan med redan behandlade korningar
    files_in_db = open("/home/pi/Test/files_in_db.txt", "a")
    files_in_db.write(file_name)
    files_in_db.write("\n") # Sa att texten laggs som en ny rad
    files_in_db.close()
    print("Test run added to database.")
    
    move_pdf(file_name) # Flytta pdf-filen till web-servern 
    remove_csv_xml(file_name)   # Ta bort csv- och xml-filerna

def get_file_names_in_db():
    files_in_db = open("/home/pi/Test/files_in_db.txt")
    file_names = []
    for line in files_in_db:
        file_names.append(line.strip()) # Maste ta bort \n sist pa raden
    files_in_db.close()
    return file_names
    
def get_file_names():
    dirs_and_files = os.listdir(dir_path)
    files = []
    names = []
    for item in dirs_and_files:
        path_to_item = join(dir_path, item)
        if isfile(path_to_item):
            files.append(item)
        
    for file in files:
        name, type = file.split(".")
        if name not in names:
            names.append(name)
    return names        
    
def move_pdf(file_name):
    current_file_path = join(dir_path, file_name + ".pdf")
    new_file_path = join("/var/www/FlaskApp/FlaskApp/static/pdf_data", file_name + ".pdf")
    
    shutil.move(current_file_path, new_file_path)
    
def remove_csv_xml(file_name):
    csv_file_path = join(dir_path, file_name + ".csv")
    xml_file_path = join(dir_path, file_name + ".xml")
    
    os.unlink(csv_file_path)
    os.unlink(xml_file_path)
        
##################################################################
# HUVUDSKRIPT
##################################################################  
file_names_in_db = get_file_names_in_db()
file_names = get_file_names()

if  len(file_names_in_db) == 0: 
    create_new_db()
    print("New database was created.")
else:
    print("Database is already created.")

for file_name in file_names:
    if file_name not in file_names_in_db:
        add_to_db(file_name)        

###################################################################
# TESTER
###################################################################
cursor, connection = connect()

# cursor.execute("SELECT duration, ID FROM test_runs WHERE duration > 1000")
# query_result = cursor.fetchall()
# print(query_result)
# print(query_result[0])
# print(query_result[0][0])
# if query_result[0][0] == 1304:
    # print("It works!")
# for element in query_result:
    # print(element)
# for element in query_result:
    # print(element[0])

# print("*************************")
# print("*************************")
# print("*************************")
# print("*************************")
# print("*************************")  

# cursor.execute("SELECT Name FROM signals WHERE test_run_ID = %s AND Unit = %s", ('354678050749345_160621T074448Z', 'mph'))
# query_result = cursor.fetchall()
# print(query_result)

# print("*************************")
# print("*************************")
# print("*************************")
# print("*************************")
# print("*************************")     

# cursor.execute("SELECT DTC_ID, DTC_TEXT, PENDING FROM OBD_DTCs")
# query_result = cursor.fetchall()
# print(query_result) 

# print("*************************")
# print("*************************")
# print("*************************")
# print("*************************")
# print("*************************")  

# cursor.execute("SELECT DTC_ID, DTC_TEXT, PENDING FROM UDS_DTCs")
# query_result = cursor.fetchall()
# print(query_result)

# print("*************************")
# print("*************************")
# print("*************************")
# print("*************************")
# print("*************************")  

cursor.execute("SELECT * FROM test_runs")
query_result = cursor.fetchall()
print(query_result) 

connection.close()
