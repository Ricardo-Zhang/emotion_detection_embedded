import requests, time
import sqlite3
from flask import Flask
from flask import request, Response
from flask_restful import Api, Resource
from functools import wraps
import hashlib

db_path = "source/records.db"

app = Flask(__name__)
api = Api(app)

user_id = None

def check_auth(username, password):
    global user_id
    with sqlite3.connect(db_path) as conn:
        cur = conn.cursor()
        cur.execute("SELECT user_id, password_hash, salt FROM users WHERE user_name IS ?", (username, ))
        user_id, saved_hash, salt = cur.fetchone()
        password_hash = hashlib.pbkdf2_hmac('md5',password.encode(),salt,100000)
    return password_hash == saved_hash


def authenticate():
    """
    Sends a 401 response that enables basic auth
    """
    return Response(
        'Could not verify your access level for that URL.\n'
        'You have to login with proper credentials', 401,
        {'WWW-Authenticate': 'Basic realm="Login Required"'})


def requires_auth(f):
    @wraps(f)
    def decorated(*args, **kwargs):
        auth = request.authorization
        if not auth or not check_auth(auth.username, auth.password):
            return authenticate()
        return f(*args, **kwargs)
    return decorated


@app.route("/")
def hello():
    return "Hello World!"


class photo(Resource):
    # @requires_auth
    def post(self):
        face_api_url = 'https://westcentralus.api.cognitive.microsoft.com/face/v1.0/detect'
        data = request.files
        data = data['image']
        image = data.read()
        print(type(image))
        subscription_key = "c27e306eb0b64d8f8f06bced4325a294"
        assert subscription_key
        headers = {'Content-Type': 'application/octet-stream',
                   'Ocp-Apim-Subscription-Key': subscription_key}
        params = {
            'returnFaceId': 'true',
            'returnFaceLandmarks': 'false',
            'returnFaceAttributes': 'age,gender,headPose,smile,facialHair,glasses,emotion',
        }
        azure_response = requests.post(face_api_url,params=params,headers=headers,data=image)
        faces = azure_response.json()
        if len(faces) == 0:
            return {'received:': 0, 'emotion': None}
        emotion_scores = faces[0]['faceAttributes']['emotion']
        max_score = 0
        print(faces)
        for emotion, score in emotion_scores.items():
            if score > max_score:
                max_score = score
                emotion_detected = emotion
        with sqlite3.connect(db_path) as conn:
            cur = conn.cursor()
            tmp = (user_id, emotion_detected, time.ctime())
            cur.execute("INSERT INTO records (user_id, emotion, time_stamp)"\
                         "VALUES (?,?,?);", tmp)
        return {'received:': 1, 'emotion': emotion_detected}


api.add_resource(photo, '/photo')

if __name__ == '__main__':
    app.run(host="0.0.0.0",debug=True)
