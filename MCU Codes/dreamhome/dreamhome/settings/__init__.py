import sys,os

print()
print()
SERVER = 	'PRODUCTION' if (os.environ.get('sl_PROD') is not None and os.environ.get('sl_PROD')=='True') else (
			'DEVELOPEMENT' if (os.environ.get('sl_DEV') is not None and os.environ.get('sl_DEV')=='True') else
			'LOCAL')
print(f"SERVER =====> {SERVER}")

if os.environ.get('sl_PROD') is not None and os.environ.get('sl_PROD')=='True':
    from dreamhome.settings.prod import * 
elif os.environ.get('sl_DEV') is not None and os.environ.get('sl_DEV')=='True':
    from dreamhome.settings.dev import *
else:
	from dreamhome.settings.local import *     



print(f"**** DEBUG STATE **** : {DEBUG}")
print(f"**** TRACEBACK **** : {'OFF' if TRACEBACK_OFF==True else 'ON'}")

print()
print("LOCAL DATABASE PROFILE : ")
print(f"**** DATABASE NAME **** : {DATABASES['local']['NAME']}")
print()


print("DEFAULT DATABASE PROFILE : ")
print(f"**** DATABASE HOST **** : {DATABASES['default']['HOST']}")
print(f"**** DATABASE NAME **** : {DATABASES['default']['NAME']}")
print(f"**** DATABASE PORT **** : {DATABASES['default']['PORT']}")
print(f"**** DATABASE USER **** : {DATABASES['default']['USER']}")
print()
print(f"**** STATIC_VERSION **** : {STATIC_VERSION}")
print()
print()
print()

if (PRINT_EANABLED==False) or ((PRODUCTION==True) and (os.environ.get('sl_SHELL') is None or os.environ.get('sl_SHELL')=='False')):
	print("REDIRECTING ALL STDOUT TO DEVNULL")
	print()
	sys.stdout = open(os.devnull, 'w')
	pass
else:	
	print("WARNING: print CAN CAUSE BROKEN PIPE")
	print("SET sl_PRINT TO FALSE (RECOMENDED)")
