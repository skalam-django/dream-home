import requests
from bs4 import BeautifulSoup

cookies = {
    'PHPSESSID': '0g7ek5d6ek0opfdpf5q8u334k7',
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
    'Referer': 'https://resident.uidai.gov.in/verify',
    'Accept-Language': 'en-GB,en-US;q=0.9,en;q=0.8',
}

params = (
    ('width', '100'),
    ('height', '40'),
    ('characters', '5'),
)

response = requests.get('https://resident.uidai.gov.in/CaptchaSecurityImages.php', headers=headers, params=params, cookies=cookies)

with open("aadhar-captcha.jpeg", "wb") as f:
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


headers = {
    'sec-ch-ua': '"Google Chrome";v="87", " Not;A Brand";v="99", "Chromium";v="87"',
    'sec-ch-ua-mobile': '?0',
    'Upgrade-Insecure-Requests': '1',
    'Origin': 'https://resident.uidai.gov.in',
    'Content-Type': 'application/x-www-form-urlencoded',
    'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.141 Safari/537.36',
    'Referer': 'https://resident.uidai.gov.in/verify',
}
data = {
  'uidno': '823466482082',
  'security_code': solve_captcha('aadhar-captcha.jpeg'),
  'form_action': 'Proceed to Verify',
  'task': 'verifyaadhaar',
  'boxchecked': '0',
  'c597d422d5bb72782694abe33b327dc5': '1'
}

data['security_code'] = 'jvMnq'


response = requests.post('https://resident.uidai.gov.in/verify', headers=headers, data=data, cookies={'PHPSESSID':'0g7ek5d6ek0opfdpf5q8u334k7'})
soup=BeautifulSoup(response.content, "lxml")
s = soup.find("div", id="maincontent")
s = s.find("div", class_="row")
s = s.find("div", class_="mb-15")
if s is not None:
  s = s.find("div", class_="pl-0")
  s = s.find("div", class_="pt-10").text




