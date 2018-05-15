# Technology Stack
### Backend Service
**HTTP Restful** server was implemented with Python **Flask** framework to handle
the **POST** requests from the emotion-tracker and the **GET** requests from the
music player.
### Emotion Detection Service
Face API from **Microsoft Azure** was
integrated in the backend to provide analysis of emotion from photo of faces.
### Data Service
User data is stored in the relational **SQLite** database containing
both basic user information and emotion status records.
### Network Security
HTTP Basic Authentication was implemented to ensure the safety of your data. Passwords are hashed with salt using the md5 algorithm and saved in the database.
### Deploy
The entire system is deployed on **Digital Ocean** cloud utilizing **Docker Container** technology.
