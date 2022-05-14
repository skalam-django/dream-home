# Dream Home

Dream Home: It's a Room Automation,

### Repo owner or admin

Sk Khurshid Alam

### Dependencies
* [**Python**](https://www.python.org/downloads/)
* [**Django**](https://docs.djangoproject.com/en/4.0/)
* [**Celery**](https://docs.celeryq.dev/en/stable/getting-started/introduction.html)
* [**PostgreSQL**](https://www.postgresql.org/download/)
* [**Redis**](https://redis.io/download/)
* [**daphne**](https://pypi.org/project/daphne/0.8.1/)
* [**NodeMCU**](https://nodemcu.readthedocs.io/en/release/)
* [**ESP32**](https://www.espressif.com/en/products/socs/esp32)
* [**Arduino IDE**](https://www.arduino.cc/en/software)

## Run the server

Install PostgreSQL, you may follow the steps given in this link:
```
https://www.cherryservers.com/blog/how-to-install-and-setup-postgresql-server-on-ubuntu-20-04
```

Install Redis-Server:
```
sudo apt-get install redis-server
```

To start Redis automatically when your server boots:
```
sudo systemctl enable redis
```

Start Redis as Service:
```
sudo systemctl start redis
```


Create Virtual Enviroment:
```
virtualenv -p python3 venv
```

Activate the Virtual Environment:
```
source venv/bin/activate
```

Install Dependecies:
```
pip install -r requirements.txt
```

Create database in your PostgreSQL server:
```
create database dreamhome;
```


Export Enviroment Variables:
```
export DH_DEV=True
export DH_PRINT=False
export DH_SERVICES=True
export DH_SECRET_KEY=9ewkumuw^0-k+_xij(g^byzycpxo!j(xy6-hy^dh#%8*4#ik5r
export DH_SSL=False
export DH_NAME=dreamhome
export DH_USER=postgres
export DH_PASSWORD=admin@123
export DH_HOST=localhost
export DH_PORT=5432
```

Migrate models into database:
```
python manage.py migrate
```

Run Application directly in Linux Terminal:
```
daphne dreamhome.asgi:application --access-log 1 --ping-interval 5 --ping-timeout 10 --port 8000 --bind localhost -v2
```
**Note:** Change host and port according to your needs.<br/>

Run Celery directly in Linux Terminal:
```
celery -A dreamhome worker -B --loglevel=info
```

## Sample Test
http://localhost:8000

## Enable Appliance-NodeMCU to communicate with Master (ESP32)
Step1: Using Arduino IDE upload appliance_credentials.ino (MCU Code/appliance_credentials/appliance_credentials.ino) to NodeMCU. This will write the credentials in EEPROM

Step2: Using Arduino IDE upload ApplianceClient.ino (MCU Code/ApplianceClient/ApplianceClient.ino) in that NodeMCU again. This is the code which communicates with Master and the Appliances

## Enable Operator-NodeMCU to communicate with Master (ESP32)
Step1: Using Arduino IDE upload operator_credentials.ino (MCU Code/operator_credentials/operator_credentials.ino) to NodeMCU

Step2: Using Arduino IDE upload OperatorClient.ino (MCU Code/OperatorClient/OperatorClient.ino) in that NodeMCU again. This is the code which communicates with Master and the Operators/Sensors

## Enable Master-ESP32 to communicate with Slaves (Appliance-NodeMCU and Operator-NodeMCU) and Server
Step1: Using Arduino IDE upload master_credentials.ino (MCU Code/master_credentials/master_credentials.ino) to NodeMCU

Step2: Using Arduino IDE upload OperatorClient.ino (MCU Code/MasterSocketServer/MasterSocketServer.ino) in that NodeMCU again. This is the code which communicates with Slaves(Operators and Appliances) and the Server




