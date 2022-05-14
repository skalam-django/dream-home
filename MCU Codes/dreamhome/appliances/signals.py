from django.conf import settings
from dreamhome.utils import loggerf, printf
from django.db import connection	

def popuplate_default_data(sender, **kwargs):
	from appliances.models import Device, PortInfo
	dev_obj = Device.objects.whitelist_device("866029038171163", "84:CC:A8:A9:17:09")
	slave_mac_list = ("40:F5:20:33:15:14", "84:0D:8E:82:B6:94")
	slave_port_list = (("D0", "D1", "D2" , "D3", "D4", "D5", "D6", "D7", "D8", "A0"), ("D0", "D1", "D2" , "D3", "D4", "D5", "D6", "D7", "D8", "A0"))
	PortInfo.objects.whitelist_slaves(dev_obj, slave_mac_list, slave_port_list)

def device_proc(sender, **kwargs):
	print('Creating device_proc() procedure in postgres.')
	from appliances.models import Device
	with connection.cursor() as cur:
		# try:
		# 	cur.execute(
		# 				"""
		# 				DO $$
		# 				BEGIN
		# 				    BEGIN
		# 				       CREATE EXTENSION IF NOT EXISTS "uuid-ossp";
		# 				    EXCEPTION 
		# 				    WHEN others THEN    
		# 				        RAISE INFO 'Using Superuser CREATE EXTENSION IF NOT EXISTS "uuid-ossp";';
		# 				    END;
		# 				END; 
		# 				$$ LANGUAGE plpgsql;
		# 				"""
		# 	)	
		# except Exception as e:
		# 	print("In device_proc() Postgres Error1 : ",e)	

		try:
			cur.execute(
						"""
						DO $$
						BEGIN
						    BEGIN
						       CREATE EXTENSION IF NOT EXISTS pgcrypto;
						    EXCEPTION 
						    WHEN others THEN    
						        RAISE INFO 'Using Superuser CREATE EXTENSION IF NOT EXISTS pgcrypto;';
						    END;
						END;
						$$ LANGUAGE plpgsql;    
						"""
			)	
		except Exception as e:
			print("In device_proc() Postgres Error2 : ",e)

		try:	
			cur.execute(
						f"""
						CREATE OR REPLACE FUNCTION generate_device() RETURNS TEXT[] AS $$
						DECLARE
							characters TEXT := 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
							size1 INT := 19;
							size2 INT := 8;
							bytes1 BYTEA := gen_random_bytes(size1);
							bytes2 BYTEA := gen_random_bytes(size2);
							l INT := length(characters);
							i INT := 0;
							vssid TEXT := NULL;
							vpass TEXT := NULL;
							exist1 BOOLEAN := TRUE;
							exist2 BOOLEAN := TRUE;
							try_counter INTEGER := 0;
						BEGIN

						  	LOOP
						  		vssid := 'Dream Home: ';
								WHILE i < size1 LOOP
									vssid := vssid || substr(characters, get_byte(bytes1, i) % l + 1, 1);
									i := i + 1;
								END LOOP;
								SELECT EXISTS (SELECT * FROM {Device._meta.db_table} WHERE ssid=vssid) INTO exist1;
								EXIT WHEN exist1 = FALSE OR try_counter >= 100;
								try_counter := try_counter + 1;
						  	END LOOP;

						  	i := 0;
						  	try_counter := 0;
						  	
						  	LOOP
						  		vpass := '';
								WHILE i < size2 LOOP
									vpass := vpass || substr(characters, get_byte(bytes2, i) % l + 1, 1);
									i := i + 1;
								END LOOP;
								SELECT EXISTS (SELECT * FROM {Device._meta.db_table} WHERE password=vpass) INTO exist2;
								EXIT WHEN exist2 = FALSE OR try_counter >= 100;
								try_counter := try_counter + 1;
						  	END LOOP;

							IF exist1 = FALSE AND exist2 = FALSE THEN
								RETURN ARRAY[vssid, vpass];
							ELSE
								RETURN ARRAY[NULL, NULL];		
							END IF;
						END;
						$$ LANGUAGE plpgsql;
						"""
				)
		except Exception as e:
			print('In device_proc() Postgres Error3 :',e)	
