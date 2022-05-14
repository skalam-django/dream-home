function FindPlaces(q, callback){
	var url = `https://www.astrosage.com/kundli/FindPlaces.asp?q=${q}`;
	var xhr = new XMLHttpRequest();
	xhr.open("GET", url);
	xhr.setRequestHeader("authority", "www.astrosage.com");
	xhr.setRequestHeader("sec-ch-ua", "'Google Chrome';v='87', ' Not;A Brand';v='99', 'Chromium';v='87'");
	xhr.setRequestHeader("accept", "*/*");
	xhr.setRequestHeader("x-requested-with", "XMLHttpRequest");
	xhr.setRequestHeader("sec-ch-ua-mobile", "?0");
	xhr.setRequestHeader("user-agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.141 Safari/537.36");
	xhr.setRequestHeader("sec-fetch-site", "same-origin");
	xhr.setRequestHeader("sec-fetch-mode", "cors");
	xhr.setRequestHeader("sec-fetch-dest", "empty");
	xhr.setRequestHeader("referer", "/");
	xhr.setRequestHeader("accept-language", "en-GB,en-US;q=0.9,en;q=0.8");
	// xhr.setRequestHeader("cookie", "__cfduid=d6259dd545f1283d7eda29dcc61edf8b61610223072; oldurl=%2FDefault%2Easp; ASPSESSIONIDCABBCSRR=GNEEBAOAKHALFDJDCEJIKEAH; __utma=170720432.1677428412.1610223075.1610223075.1610223075.1; __utmc=170720432; __utmz=170720432.1610223075.1.1.utmcsr=(direct)|utmccn=(direct)|utmcmd=(none); lastadvshowonday=10; lastadvshowonmonth=1; lastadvshowonyear=2021; ishowadv6=false; __gads=ID=ec936f05c26e1378:T=1610223646:S=ALNI_MbCjVwTACYjMCKy6mlChaqh_CGE4A; __cf_bm=8e5c71851084567b24bf6ab5d1a2396c0003821f-1610225762-1800-AZUtndtWIqdvDCxQDKWk4tn4Am81YDR2om15z2R6x8JRZp3IEkjbz+2VVgQ3Cwt+0Kjn/w+q1OGhNuQqKSUYafc=; __utmb=170720432.10.9.1610223640736");
	xhr.onreadystatechange = function () {
	   if (xhr.readyState === 4) {
	      if (xhr.status==200){
	      	callback(parseData(xhr.responseText));
	      }
	   }};

	xhr.send();
}

function parseData(data) {
    if (!data) return null;
    var parsed = [];
    var rows = data.split("\n");
    for (var i = 0; i < rows.length; i++) {
        var row = $.trim(rows[i]);
        if (row) {
            parsed[parsed.length] = row.split("|");
        }
    }
    return parsed;
};