var BASE_JS = (function() {
	if ('serviceWorker' in navigator) {
	  navigator.serviceWorker
	    .register('/sw.js')
	    .then(function() {
	      console.log('Service worker registered!');
	    });
	}

	window.addEventListener('beforeinstallprompt', function(event) {
		console.log('beforeinstallprompt fired');
		event.userChoice.then(function(choiceResult) {
			if (choiceResult.outcome === 'dismissed') {
				console.log('User cancelled manifest installation');
			} else {
				console.log('User added to home screen');
			}
		});
		return true;
	});

	// function show_msg(msg, field=null, timeout=3000){
	// 	if (msg && msg!=""){
	// 		if (field){
	// 			msg = msg.replace(field, `<strong>${field}</strong>`)
	// 		}

	// 		$("#snackbar-id").html(msg);
	// 		$(".mdc-snackbar").css("display", "block");
	// 		$(".mdc-snackbar__surface").css("opacity", 1);
	// 		setTimeout(function(){
	// 			$("#snackbar-id").text("");
	// 			$(".mdc-snackbar").css("display", "none");
	// 			$(".mdc-snackbar__surface").css("opacity", 0);					
	// 		}, timeout);
	// 	}
	// }
  var showToast=function(idx=0, msg=""){
    var notification = document.querySelector(`#snackbar${idx}`);
    notification.MaterialSnackbar.showSnackbar(
      {
        message: msg
      }
    );     
  }
  var loading = function(idx=0, state=true){
    if (state==true){
      document.querySelector(`#loader${idx}`).style.display='block';
    }else{
      document.querySelector(`#loader${idx}`).style.display='none';
    }
  }

	function is_valid_value(val){
	    if ((val instanceof Object==true && jQuery.isEmptyObject(val)==true) || (val instanceof Object==false && typeof val!="string" && isNaN(val)==true) || val==undefined || val==null)
	        return false;
	    return true;
	} 


	const parseBool = (value, defaultValue) => ['true', 'false', true, false].includes(value) && JSON.parse(value) || defaultValue;



	var isMobile = {
	    Android: function() {
	        return navigator.userAgent.match(/Android/i);
	    },
	    BlackBerry: function() {
	        return navigator.userAgent.match(/BlackBerry/i);
	    },
	    iOS: function() {
	        return navigator.userAgent.match(/iPhone|iPad|iPod/i);
	    },
	    Opera: function() {
	        return navigator.userAgent.match(/Opera Mini/i);
	    },
	    Windows: function() {
	        return navigator.userAgent.match(/IEMobile/i);
	    },
	    any: function() {
	        return (isMobile.Android() || isMobile.BlackBerry() || isMobile.iOS() || isMobile.Opera() || isMobile.Windows());
	    }
	};

	var detect_browser =function()
	{
	    var browser_dict = new Object;
	    browser_dict['Opera']   =   [30,'b4096','b4096'];
	    browser_dict['OPR']     =   [70,'b4096','b4096000'];
	    browser_dict['Chrome']  =   [70,'b4096','b4096000'];
	    browser_dict['Safari']  =   [600,'b4096','b4096'];
	    browser_dict['Firefox'] =   [50,'c4097','c4097000'];
	    browser_dict['MSIE']    =   [20,'c4096','c4096'];

	    for (var b in browser_dict)
	        if (navigator.userAgent.indexOf(b) != -1)
	            return {"name":b, "values":browser_dict[b]};
	    if (!!document.documentMode == true)
	        return {"name":"MSIE", "values":browser_dict["MSIE"]};
	}


  var SmartAjax = function(options)
  {
      var vars = {
          url             :   undefined,
          protocol        :   'POST',
          accept          :   'application/json; charset=utf-8',
          content_type    :   'application/json; charset=utf-8',
          authorization   :   undefined,
          dataType        :   'json',
          is_async        :   true,
          searchParams    :   new Object(),
          json_data       :   new Object(),
          interval        :   5000,
          inactive_mins   :   undefined,
          time_out        :   5,
          max_time_out    :   10,
          time_out_msg    :   'Timeout occured, please try again!',
          once            :   true, 
          app             :   "Web",
          success_callback: function(data){
              console.error('SmartAjax() error_callback() Error: ', data);
          },  
          error_callback  : function(e){
              console.error('SmartAjax() error_callback() Error: ', e);
          },
      }
      sa_root = this;
      var construct = function(options){
          vars = Object.assign(vars, options);
          if (vars.inactive_mins==undefined)
              vars.inactive_mins  =   (1.2*vars.interval)/(60*1000);

          sa_root.started_at    =   new Date();
          sa_root.restarted_at  =   new Date();
          sa_root.st_time_out   =   new Date();
          sa_root.reseted       =   true;
          sa_root.active        =   true;
          sa_root.delay         =   500;
          var events          =   ['mousedown', 'mousemove', 'keypress', 'scroll', 'touchstart'];

          events.forEach(function(e) {
              try{
                if (typeof(sa_root) != 'undefined' && vars.once==false){
                  document.addEventListener(e, resetTimeOut, true);
                }
              }catch(e){

              }
          });
      };

      sa_root.get_update = function()
      {
          var durations = undefined;
          var a = ajax();
          if (a[0]==false){
            console.log('SmartAjax() error_callback() Error: url not defined --> url: ',vars.url);
            vars.error_callback(a[1]);
          }
                  
          if(vars.once==false){
              sa_root.intv = setInterval(function(){
                  sa_root.tmout = setTimeout(function(){ 
                  var ed_time_out = new Date();
                      if (document.hidden==false)
                          resetTimeOut();
                      durations = (ed_time_out.getTime()-sa_root.st_time_out.getTime())/(60*1000);
                      var max_time_out = (ed_time_out.getTime()-sa_root.started_at.getTime())/(60*1000);
                      
                      if (max_time_out > vars.max_time_out){
                          if(vars.time_out_msg!=undefined)
                              showToast(2, vars.time_out_msg)  
                          sa_root.stop_update();
                      }
                      if ((ed_time_out.getTime()-sa_root.restarted_at.getTime())/(60*1000) > vars.time_out){                       
                          if(vars.time_out_msg!=undefined){
                              showToast(2, vars.time_out_msg)
                          }                                                         
                          sa_root.stop_update();
                      }
                      if ((durations < vars.inactive_mins) || sa_root.reseted==true){
                          sa_root.reseted = false;
                          sa_root.active = true;
                          var a = ajax();
                          if (a[0]==false){
                            console.log(`SmartAjax().ajax() Error: ${a[1]}`);
                            vars.error_callback(a[1]);
                          }
                      }
                      else{
                          sa_root.active = false;
                      }

                  }, sa_root.delay);
                  sa_root.delay += 500;
              },vars.interval);
              return sa_root.intv;
          }
      }

      sa_root.stop_update =function(){
          clearTimeout(sa_root.tmout);
          clearInterval(sa_root.intv);
      }
      var ajax = function()
      {
          try
          {
          	  var csrf_token = document.querySelector('input[name=csrfmiddlewaretoken]').value;

              if(vars.url==undefined || vars.url==null || vars.url==NaN)
                  return [false,`url not defined --> url: ${vars.url}`];
              var xhr = new XMLHttpRequest();
              xhr.timeout = 10000;

              try{
                var url = new URL(vars.url);
              }catch(e){
                var url = new URL(`${location.origin}${vars.url}`);
              }
              
              for(var key in vars.searchParams){
                url.searchParams.set(key, vars.searchParams[key]);
              }
              xhr.open(vars.protocol, url, vars.is_async);
              xhr.responseType =  vars.dataType;
              xhr.setRequestHeader('Accept', vars.accept);
              xhr.setRequestHeader('Content-type', vars.content_type);
              xhr.setRequestHeader('Authorization', vars.authorization);
              xhr.setRequestHeader('X-CSRFToken', csrf_token);
              xhr.setRequestHeader('App', vars.app);
              // console.log('csrf_token: ',csrf_token)
              xhr.onload = function() {
                if (vars.dataType=='json'){
                  var response = xhr.response;
                }else{
                  var response = xhr.responseText;
                }
                if (xhr.status>=200 && xhr.status<500){
                  vars.success_callback(response);
                }else{
                  vars.error_callback(response);
                }
              };

              xhr.onerror = function() {
                console.log("onerror");
                vars.error_callback("Network down");
              };

              xhr.onreadystatechange = function() {
                if (xhr.readyState <= 3) {
                  
                }
                if (xhr.readyState == 4) {
                  
                }
              };
              var json_data = null;
              if (vars.json_data!=null && vars.json_data!= undefined && vars.json_data!={}){
                json_data = typeof(vars.json_data)=="string" ? vars.json_data : JSON.stringify(vars.json_data);
              }
              // console.log("json_data: ", json_data)
              xhr.send(json_data);
              return [true];
          }
          catch(e)
          {
              return [false, e];
          }
      }
      var resetTimeOut = function()
      {
          sa_root.st_time_out   =  new Date();
          sa_root.restarted_at  =  new Date();
          sa_root.reseted       =  true;
          sa_root.delay         =  1000;
          if (sa_root.active==false){
              sa_root.started_at =   new Date();
          }
      }

      construct(options);
  }


  var getUrlParameter = function getUrlParameter(sParam) {
      var sPageURL = window.location.search.substring(1),
          sURLVariables = sPageURL.split('&'),
          sParameterName,
          i;

      for (i = 0; i < sURLVariables.length; i++) {
          sParameterName = sURLVariables[i].split('=');

          if (sParameterName[0] === sParam) {
              return sParameterName[1] === undefined ? true : decodeURIComponent(sParameterName[1]);
          }
      }
  };


  const scale = (num, in_min, in_max, out_min, out_max) => {
    return (num - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }

  const decimal_format = (val, decimal_point) => {
    return parseFloat(`${(Number(Math.round(parseFloat(val) + 'e' + decimal_point) + 'e-' + decimal_point)).toFixed(decimal_point)}`);    
  }


	return {
			// show_msg:show_msg,
			is_valid_value:is_valid_value,
			parseBool:parseBool,
			isMobile:isMobile,
			detect_browser:detect_browser,
			SmartAjax : SmartAjax,
      showToast : showToast,
      loading : loading,
      getUrlParameter : getUrlParameter,
      scale : scale,
      decimal_format : decimal_format,
		}
})()

