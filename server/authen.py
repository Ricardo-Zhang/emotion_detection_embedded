import os
import hashlib
import sqlite3


class User(object):
    def __init__(self, user_name, password):
        self.name = user_name

    def add_salt(self, passcode):
        self.salt = os.urandom(32)
        self.hash = hashlib.pbkdf2_hmac('md5', self.password.encode(), self.salt, 100000)

    # def check_user(self):
        # return self.add_salt()

    def save_user(self):
        tmp = (self.name, self.hash, self.salt)
        print(tmp)
        with sqlite3.connect("source/records.db") as conn:
            cur = conn.cursor()
            cur.execute("INSERT INTO users (user_name, password_hash, salt)"\
                         "VALUES (?,?,?);", tmp)
            conn.commit()

def __main__():
    user = User('zz524', 'hikuh8fg96t')
    user.add_salt()
    user.save_user()
    
