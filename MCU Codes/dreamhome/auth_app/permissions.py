from django.conf import settings

def get_groups(user_type_arr, user_group_tpl_arr):
    group_arr = []
    for idx, user_type in enumerate(user_type_arr):
        if user_type in settings.USER_TYPE:
            for idx1, user_group in enumerate(user_group_tpl_arr[idx]):
                if user_group in settings.USER_GROUP:
                    group_arr.append(f"{user_type}-{user_group}")
    return group_arr

MODEL_PERMS =   {
                    
                    'token' :   {
                                    'add'       :   get_groups(['dreamhome',], [('admin','staff', 'client'),]),
                                    'view'      :   get_groups(['dreamhome',], [('admin','staff', 'client'),]),
                                    'delete'    :   get_groups(['dreamhome',], [('admin','staff', 'client'),]),
                    },
                    'authuser' : {
                                    'add'       :   get_groups(['dreamhome',], [('admin','staff', 'client'),]),
                                    'change'    :   get_groups(['dreamhome',], [('admin','staff', 'client'),]),
                                    'view'      :   get_groups(['dreamhome',], [('admin','staff', 'client'),]),
                                    'delete'    :   get_groups(['dreamhome',], [('admin','staff', 'client'),]),

                    },
                    'permission' : {
                                    'add'       :   get_groups(['dreamhome',], [('admin','staff', 'client'),]),
                                    'change'    :   get_groups(['dreamhome',], [('admin','staff', 'client'),]),
                                    'view'      :   get_groups(['dreamhome',], [('admin','staff', 'client'),]),
                                    'delete'    :   get_groups(['dreamhome',], [('admin','staff', 'client'),]),
                    },    
                    'group' : {
                                    'add'       :   get_groups(['dreamhome',], [('admin','staff', 'client'),]),
                                    'change'    :   get_groups(['dreamhome',], [('admin','staff', 'client'),]),
                                    'view'      :   get_groups(['dreamhome',], [('admin','staff', 'client'),]), 
                                    'delete'    :   get_groups(['dreamhome',], [('admin','staff', 'client'),]),                  
                    },    
                    'contenttype' : {
                                    'add'       :   get_groups(['dreamhome',], [('admin','staff', 'client'),]),
                                    'change'    :   get_groups(['dreamhome',], [('admin','staff', 'client'),]),
                                    'view'      :   get_groups(['dreamhome',], [('admin','staff', 'client'),]),
                                    'delete'    :   get_groups(['dreamhome',], [('admin','staff', 'client'),]),
                    },                                                                                                                      
                }





