
from auth_app.models import UserTracking

class DbRouter(object):
    def db_for_read(self, model, **hints):
        if model in [UserTracking]:
            return "local"
        return "default"

    def db_for_write(self, model, **hints):
        if model in [UserTracking]:
            return "local"
        return "default"

    def allow_relation(self, obj1, obj2, **hints):
        return True

    def allow_migrate(self, db, app_label, model_name=None, **hints):
        print("model_name: ", model_name)
        if model_name in ["usertracking"]:
            return db == "local"   
        return db == "default"
