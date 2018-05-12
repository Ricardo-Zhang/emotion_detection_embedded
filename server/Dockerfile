FROM ubuntu:latest

#Update OS
RUN sed -i 's/# \(.*multiverse$\)/\1/g' /etc/apt/sources.list
RUN apt-get update
RUN apt-get -y upgrade

# Install Python
RUN apt-get install -y python-dev python-pip

# Add requirements.txt
ADD requirements.txt /webapp

# Install uwsgi Python web server
RUN pip install uwsgi

# Install app requirements
RUN pip install -r app/requirements.txt

# Create app directory
ADD . /app

# Set the default directory for our environment
ENV HOME /app
WORKDIR /app

# Expose port 8000 for uwsgi
EXPOSE 80


ENTRYPOINT ["uwsgi", "--http", "0.0.0.0:80", "--module", "app:app", "--processes", "1", "--threads", "8"]
CMD ["app.py"]
