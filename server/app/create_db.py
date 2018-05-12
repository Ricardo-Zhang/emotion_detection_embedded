import sqlite3

conn = sqlite3.connect('source/records.db')

conn.execute('''CREATE TABLE users
             (user_id INTEGER PRIMARY KEY, user_name TEXT, password_hash TEXT, salt TEXT)''')
conn.execute('''CREATE TABLE records
             (record_id INTEGER PRIMARY KEY, user_id TEXT, emotion TEXT, time TIMESTAMP)''')
conn.close()