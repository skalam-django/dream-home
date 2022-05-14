# Voter ID:

	# https://electoralsearch.in/##resultArea

import requests

cookies = {
    '__RequestVerificationToken': 'FzbpJyVT0j_e3iRwwyxM7o5tWzckWN1USTiYqH2U4qoV_OYUQ7rpsJkUhaGPUXHuhDSDvV9tctJiwrg70DnyCxCdA75KFNrYswzzdVe2iPQ1',
    'ServerAffinity': 'f00f9df1c11c099e277c36bf19636e9da296f4865cefbd3fdcd84ee015b8edaa',
    'runOnce': 'true',
    'electoralSearchId': 'mkiqhn33ehaxey2ttrn5bpzo',
}

headers = {
    'Connection': 'keep-alive',
    'sec-ch-ua': '"Google Chrome";v="87", " Not;A Brand";v="99", "Chromium";v="87"',
    'sec-ch-ua-mobile': '?0',
    'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.141 Safari/537.36',
    'Accept': 'image/avif,image/webp,image/apng,image/*,*/*;q=0.8',
    'Sec-Fetch-Site': 'same-origin',
    'Sec-Fetch-Mode': 'no-cors',
    'Sec-Fetch-Dest': 'image',
    'Referer': 'https://electoralsearch.in/',
    'Accept-Language': 'en-GB,en-US;q=0.9,en;q=0.8',
}

params = (
    ('image', 'true'),
    ('id', 'Tue Jan 26 2021 15:27:28 GMT 0530 (India Standard Time)'),
)

response = requests.get('https://electoralsearch.in/Home/GetCaptcha', headers=headers, params=params, cookies=cookies)


with open("voter-captcha.jpg", "wb") as f:
  f.write(response.content)


import pytesseract
import os
import argparse
from PIL import Image, ImageOps, ImageEnhance, ImageFile
ImageFile.LOAD_TRUNCATED_IMAGES = True

def solve_captcha(path):
    image = Image.open(path).convert('RGB')
    image = ImageOps.autocontrast(image)
    filename = "{}.png".format(os.getpid())
    image.save(filename)
    text = pytesseract.image_to_string(Image.open(filename))
    return text


cookies = {
    '__RequestVerificationToken': 'FzbpJyVT0j_e3iRwwyxM7o5tWzckWN1USTiYqH2U4qoV_OYUQ7rpsJkUhaGPUXHuhDSDvV9tctJiwrg70DnyCxCdA75KFNrYswzzdVe2iPQ1',
    'ServerAffinity': 'f00f9df1c11c099e277c36bf19636e9da296f4865cefbd3fdcd84ee015b8edaa',
    'runOnce': 'true',
    'electoralSearchId': 'mkiqhn33ehaxey2ttrn5bpzo',
}

headers = {
    'Connection': 'keep-alive',
    'sec-ch-ua': '"Google Chrome";v="87", " Not;A Brand";v="99", "Chromium";v="87"',
    'Accept': 'application/json, text/plain, */*',
    'sec-ch-ua-mobile': '?0',
    'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.141 Safari/537.36',
    'Sec-Fetch-Site': 'same-origin',
    'Sec-Fetch-Mode': 'cors',
    'Sec-Fetch-Dest': 'empty',
    'Referer': 'https://electoralsearch.in/',
    'Accept-Language': 'en-GB,en-US;q=0.9,en;q=0.8',
}

params = (
    ('epic_no', 'TXV1838409'),
    ('page_no', '1'),
    ('results_per_page', '10'),
    ('reureureired', 'ca3ac2c8-4676-48eb-9129-4cdce3adf6ea'),
    ('search_type', 'epic'),
    ('state', 'S25'),
    ('txtCaptcha', solve_captcha('voter-captcha.jpg')),
)

response = requests.get('https://electoralsearch.in/Home/searchVoter', headers=headers, params=params, cookies=cookies)


	Reponses : 

	Wrong Captcha

	{
	  "response": {
	    "docs": [],
	    "numFound": 0,
	    "start": 0
	  }
	}


	{
	  "response": {
	    "docs": [
	      {
	        "pc_name": "ULUBERIA",
	        "st_code": "S25",
	        "ps_lat_long_1_coordinate": 0,
	        "gender": "M",
	        "rln_name_v2": "",
	        "rln_name_v1": "সেখ সিরাজুল হক",
	        "rln_name_v3": "",
	        "name_v1": "সেখ খুরশিদ আলম",
	        "epic_no": "TXV1838408",
	        "ac_name": "Uluberia Uttar",
	        "name_v2": "",
	        "name_v3": "",
	        "ps_lat_long": "0.0,0.0",
	        "pc_no": "26",
	        "last_update": "Thu Jan 21 04:41:15 IST 2021",
	        "id": "S251770141010701",
	        "dist_no": "14",
	        "ps_no": "141",
	        "ps_name": "Magurkhali",
	        "ps_name_v1": "মাগুরখালী",
	        "st_name": "West Bengal",
	        "dist_name": "HOWRAH",
	        "rln_type": "F",
	        "pc_name_v1": "উলুবেড়িয়া",
	        "part_name_v1": "বাহির চন্দনপুর প্রাঃ স্কুল ",
	        "ac_name_v1": "উলুবেড়িয়া উত্তর",
	        "part_no": "141",
	        "dist_name_v1": "হাওড়া",
	        "ps_lat_long_0_coordinate": 0,
	        "_version_": 1689468861464707074,
	        "name": "SK KHURSHID ALAM",
	        "section_no": "1",
	        "ac_no": "177",
	        "slno_inpart": "701",
	        "rln_name": "SK SIRAJUL HAQUE",
	        "age": 28,
	        "part_name": "Bahir Chandanpur Pry School "
	      }
	    ],
	    "numFound": 1,
	    "start": 0
	  }
	}



