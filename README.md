# Dream Home

Dream Home: It's a Room Automation,

### Repo owner or admin

Sk Khurshid Alam

### Dependencies
* [**Python**]()
* [**Django**]()
* [**Celery**]()
* [**Redis**]()
* [**dphane**]()
* [**NodeMCU**]()
* [**ESP32**]()

## Run the server

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

Export Enviroment Variables:
```
export DH_DEV=True
export DH_PRINT=False
export DH_SERVICES=True
export DH_SECRET_KEY=9ewkumuw^0-k+_xij(g^byzycpxo!j(xy6-hy^dh#%8*4#ik5r
export DH_SSL=True
export DH_NAME=dreamhome
export DH_USER=postgres
export DH_PASSWORD=admin@123
export DH_HOST=localhost
export DH_PORT=5432
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