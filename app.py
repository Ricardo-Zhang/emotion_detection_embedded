import requests
from flask import Flask
from flask import request, Response
from flask_restful import reqparse, abort, Api, Resource
from functools import wraps


app = Flask(__name__)
api = Api(app)


def check_auth(username, password):
    return username == 'zz524' and password == 'pychvjkiyktrdcjvjk.l-098545wd5678913h4g1898oiuhbg9p1;i39-4g8ru'


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


class photo(Resource):
    @requires_auth
    def post(self):
        face_api_url = 'https://westcentralus.api.cognitive.microsoft.com/face/v1.0/detect'
        data = request.files
        data = data['image']
        image = data.read()
        subscription_key = "c27e306eb0b64d8f8f06bced4325a294"
        assert subscription_key
        headers = {'Content-Type': 'application/octet-stream',
                   'Ocp-Apim-Subscription-Key': subscription_key}
        params = {
            'returnFaceId': 'true',
            'returnFaceLandmarks': 'false',
            'returnFaceAttributes': 'age,gender,headPose,smile,facialHair,glasses,emotion,hair,makeup,occlusion,accessories,blur,exposure,noise',
        }
        response = requests.post(face_api_url,params=params,headers=headers,data=image)
        faces = response.json()
        if len(faces) == 0:
            return {'received:': 1, 'emotion': None}
        emotion_scores = faces[0]['faceAttributes']['emotion']
        max_score = 0
        print(faces)
        for emotion, score in emotion_scores.items():
            if score > max_score:
                max_score = score
                emotion_detected = emotion
        return {'received:': 1, 'emotion': emotion_detected}


api.add_resource(photo, '/photo')

if __name__ == '__main__':
    app.run(debug=True)
