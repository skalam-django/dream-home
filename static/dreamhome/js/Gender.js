function Gender(s)
{
while (s.substring(0,1) == ' ')
{
s = s.substring(1,s.length);
}while (s.substring(s.length-1,s.length) == ' '){s = s.substring(0,s.length-1);
}var nm=s;
if (s.length<3)
{alert("Please enter a valid Indian name"); return false;}
var i=26+25+24+22;var j=26+25+24+23+7;var res="Dont know";if ((nm.substr(nm.length-1,1).toLowerCase()==unescape('%' + j.toString(16))) || (nm.substr(nm.length-1,1).toLowerCase()==unescape('%' + i.toString(16))))	{res="female";} else {res="male";} 
var msg=(s+ " is a "+res+" name");
console.log(msg)
//var wrt=document.getElementById("result").value=msg;
}