const char* html = R"====(<!DOCTYPE html>
<html>
<head>
<meta name='viewport' content='width=device-width, initial-scale=1, user-scalable=yes'/>
<meta http-equiv='refresh' content='_RATE_; url=/'/>
<style>
body {color:#DDD;font-size:40px;background:#333;}
.button {
    background-color: #4CAF50; /* Green */
    border: none;
    color: white;
    padding: 5px;
    text-align: center;
    text-decoration: none;
    display: inline-block;
    font-size: 25px;
}
.switch {
  position: relative;
  display: inline-block;
  height: 34px;
}
.sliderWidth{  width: 60px;}
.switch input, .dropdown input {display:none;}
.selected {
-webkit-filter: grayscale(100%);
filter: grayscale(100%);
text-align:center;}
input:checked + .selected {
-webkit-filter: grayscale(0%);
filter: grayscale(0%);}
.fan, .auto{color:#0C0}
.qspeed {width:20px;height:5px;}
.speed{color:#c55;}
.speedbar{width:20px; background:#c55; display:inline-block;}
.speed1{height:05px;}
.speed2{height:15px}
.speed3{height:25px}
.speed4{height:35px}
.slider {
  position: absolute;
  cursor: pointer;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-color: #ccc;
  -webkit-transition: .4s;
  transition: .4s;
}
.slider:before {
  position: absolute;
  content: "";
  height: 26px;
  width: 26px;
  left: 4px;
  bottom: 4px;
  background-color: white;
  -webkit-transition: .4s;
  transition: .4s;
}
input:checked + .slider {
  background-color: #0c0;
}
input:focus + .slider {
  box-shadow: 0 0 1px #2196F3;
}
input:checked + .slider:before {
  -webkit-transform: translateX(26px);
  -ms-transform: translateX(26px);
  transform: translateX(26px);
}
/* Rounded sliders */
.slider.round {
  border-radius: 34px;
}
.rotate0{display: inline-block; 
}
.rotate22{display: inline-block; 
    -ms-transform: rotate(22.5deg); /* IE 9 */
    -webkit-transform: rotate(22.5deg); /* Chrome, Safari, Opera */
    transform: rotate(22.5deg);
}
.rotate45{display: inline-block; 
    -ms-transform: rotate(45deg); /* IE 9 */
    -webkit-transform: rotate(45deg); /* Chrome, Safari, Opera */
    transform: rotate(45deg);
}
.rotate57{display: inline-block; 
    -ms-transform: rotate(57deg); /* IE 9 */
    -webkit-transform: rotate(57deg); /* Chrome, Safari, Opera */
    transform: rotate(57deg);
}

.rotate67{display: inline-block; 
    -ms-transform: rotate(67.5deg); /* IE 9 */
    -webkit-transform: rotate(67.5deg); /* Chrome, Safari, Opera */
    transform: rotate(67.5deg);
}

.rotate90{display: inline-block; 
    -ms-transform: rotate(90deg); /* IE 9 */
    -webkit-transform: rotate(90deg); /* Chrome, Safari, Opera */
    transform: rotate(90deg);
}
.rotate124{display: inline-block; 
    -ms-transform: rotate(124deg); /* IE 9 */
    -webkit-transform: rotate(124deg); /* Chrome, Safari, Opera */
    transform: rotate(124deg);
}
.rotate135{display: inline-block; 
    -ms-transform: rotate(135deg); /* IE 9 */
    -webkit-transform: rotate(135deg); /* Chrome, Safari, Opera */
    transform: rotate(135deg);
}
.rotate157{display: inline-block; 
    -ms-transform: rotate(157.5deg); /* IE 9 */
    -webkit-transform: rotate(157.5deg); /* Chrome, Safari, Opera */
    transform: rotate(157.5deg);
}
.rotate180{display: inline-block; 
    -ms-transform: rotate(180deg); /* IE 9 */
    -webkit-transform: rotate(180deg); /* Chrome, Safari, Opera */
    transform: rotate(180deg);
}
.rotateV{
    position: relative;
    float: left;
    animation-name:swingV;
    animation-duration:5s;
    animation-direction: alternate;
    animation-iteration-count: infinite;
    transform-origin:left center;
}
@-moz-keyframes swingV{
    0%{-moz-transform:rotate(0deg)}
    25%{-moz-transform:rotate(22.5deg)}
    50%{-moz-transform:rotate(45deg)}
    75%{-moz-transform:rotate(67.5deg)}
    100%{-moz-transform:rotate(90deg)}
}
@-webkit-keyframes swingV{
    0%{-webkit-transform:rotate(0deg)}
    25%{-webkit-transform:rotate(22.5deg)}
    50%{-webkit-transform:rotate(45deg)}
    75%{-webkit-transform:rotate(67.5deg)}
    100%{-webkit-transform:rotate(90deg)}
}
.rotateH{
    position: relative;
    float: left;
    animation-name:swingH;
    animation-duration:5s;
    animation-direction: alternate;
    animation-iteration-count: infinite;
    transform-origin:center center;
}
@-moz-keyframes swingH{
    0%{-moz-transform:rotate(22.5deg)}
    25%{-moz-transform:rotate(57deg)}
    50%{-moz-transform:rotate(90deg)}
    75%{-moz-transform:rotate(124deg)}
    100%{-moz-transform:rotate(157.5deg)}
}
@-webkit-keyframes swingH{
    0%{-webkit-transform:rotate(22.5deg)}
    25%{-webkit-transform:rotate(57deg)}
    50%{-webkit-transform:rotate(90deg)}
    75%{-webkit-transform:rotate(124deg)}
    100%{-webkit-transform:rotate(157.5deg)}
}
.slider.round:before {
  border-radius: 50%;
}
.hidden {visibility:hidden}
.dropbtn {
    background-color: transparent;
    color: white;
    border: none;
    cursor: pointer;
}
.dropdown {
    position: relative;
    display: inline-block;
}
.dropdown-content {
    display: none;
    position: absolute;
    background-color: #333;
    padding: 5px;
    box-shadow: 0px 8px 16px 0px rgba(0,0,0,0.2);
    z-index: 1;
}
.dropdown-content label {
    color: white;
    text-decoration: none;
    display: block;
}
.dropdown:hover .dropdown-content {
    display: block;
}
table {width:100%;}
table tr td:first-child{width:65px;}
</style>
<script>
function changeVane(id,cls,txt,val)
{
  document.getElementById(id+"_").className=cls;
  document.getElementById(id+"_").innerHTML=txt;
  document.getElementById(id).value=val;
  document.getElementById("F"+id+"_").submit();
}
function setTemp(b)
{
  var t = document.getElementById('TEMP');
  if(b && t.value < 31)
   { t.value++; }
  else if(!b && t.value > 16)
   { t.value--; }
  document.getElementById("FTEMP_").submit();
}
</script>
</head>
<body>
<table>
<tr>
<td>&#x1f321;</td><td>_ROOMTEMP_&deg;C</td> 
<tr>
<td>&#9889;&#65039;</td>
<td> 
  <form id="form" onchange="this.submit()">
  <input name="PWRCHK" type="hidden" value="">
  <label class="switch">
    <input name="POWER" type="checkbox" value="ON" _POWER_>
    <div class="sliderWidth slider round"></div>
  </label>
</form>
</td>
</tr>
<tr>
<td>&#9881;</td>
<td> 
<form onchange="this.submit()">  
<label class="switch">
  <input name="MODE" type="radio" value="AUTO" _MODE_A_>
  <div class="selected auto">&#9851;</div>
</label>
<label class="switch">
  <input name="MODE" type="radio" value="DRY" _MODE_D_>
  <div class="selected">&#128167;</div>
</label>
<label class="switch">
  <input name="MODE" type="radio" value="COOL" _MODE_C_>
  <div class="selected">&#10052;&#65039;</div>
</label>
<label class="switch">
  <input name="MODE" type="radio" value="HEAT" _MODE_H_>
  <div class="selected">&#9728;&#65039;</div>
</label>
<label class="switch">
  <input name="MODE" type="radio" value="FAN" _MODE_F_>
  <div class="selected fan">&#10051;</div>
</label>
</form>
</td>
</tr>
<tr>
<td>&#127788;</td>
<td>
<form onchange="this.submit()">  
<label class="switch">
  <input name="FAN" type="radio" value="AUTO" _FAN_A_>
  <div class="selected speed">&#9851;</div>
</label>
<label class="switch">
  <input name="FAN" type="radio" value="QUIET" _FAN_Q_>
  <div class="selected speed qspeed" style="width:20px;height:5px;">&#8230;</div>
</label>
<label class="switch"  style="">
  <input name="FAN" type="radio" value="1" _FAN_1_>
  <div class="selected speed"><div class="speedbar speed1"></div></div></div>
</label>
<label class="switch">
  <input name="FAN" type="radio" value="2" _FAN_2_>
  <div class="selected speed"><div class="speedbar speed2"></div></div>
</label>
<label class="switch">
  <input name="FAN" type="radio" value="3" _FAN_3_>
  <div class="selected speed"><div class="speedbar speed3"></div></div>
</label>
<label class="switch">
  <input name="FAN" type="radio" value="4" _FAN_4_>
  <div class="selected speed"><div class="speedbar speed4"></div></div>
</label>
</form>
</td>
</tr>
<tr>
<td>\</td>
<td>  
<div class="dropdown">
  <form id="FVANE_"><input name="VANE" id="VANE" type="text" value="_VANE_V_"/></form>
  <div class="_VANE_C_" id="VANE_">_VANE_T_</div>
  <div class="dropdown-content">
    <label><div class="" onclick="changeVane('VANE',this.className,this.innerHTML,'AUTO')">Auto</div></label>
    <label><div class="rotate0" onclick="changeVane('VANE',this.className,this.innerHTML,1)">&#10143;</div></label>
    <label><div class="rotate22" onclick="changeVane('VANE',this.className,this.innerHTML,2)">&#10143;</div></label>
    <label><div class="rotate45" onclick="changeVane('VANE',this.className,this.innerHTML,3)">&#10143;</div></label>
    <label><div class="rotate67" onclick="changeVane('VANE',this.className,this.innerHTML,4)">&#10143;</div></label>
    <label><div class="rotate90" onclick="changeVane('VANE',this.className,this.innerHTML,5)">&#10143;</div></label>
    <label><div class="" onclick="changeVane('VANE','rotateV','&#10143;','SWING')">Swing</div></label>
  </div>
</div>

</td>
</tr>
<tr>
<td>=</td>
<td> 
<div class="dropdown">
  <form id="FWIDEVANE_"><input name="WIDEVANE" id="WIDEVANE" type="text" value="_WIDEVANE_V_"/></form>
  <div class="_WIDEVANE_C_" id="WIDEVANE_">_WIDEVANE_T_</div>
  <div class="dropdown-content">
    <label><div class="rotate157" onclick="changeVane('WIDEVANE',this.className,this.innerHTML,'<<')">&#10143;</div></label>
    <label><div class="rotate124" onclick="changeVane('WIDEVANE',this.className,this.innerHTML,'<')">&#10143;</div></label>
    <label><div class="rotate90" onclick="changeVane('WIDEVANE',this.className,this.innerHTML,'|')">&#10143;</div></label>
    <label><div class="rotate57" onclick="changeVane('WIDEVANE',this.className,this.innerHTML,'>')">&#10143;</div></label>
    <label><div class="rotate22" onclick="changeVane('WIDEVANE',this.className,this.innerHTML,'>>')">&#10143;</div></label>
    <label><div class="" onclick="changeVane('WIDEVANE',this.className,this.innerHTML,'<>')">
       <div class="rotate124">&#10143;</div>&nbsp;
       <div class="rotate57">&#10143;</div>
    </div></label>
    <label><div class="" onclick="changeVane('WIDEVANE','rotateH','&#10143;','SWING')">Swing</div>
  </div>
</div>
</td>
</tr>
<tr>
<td>&#x1f321;</td>
<td> 
<input class="button" type='button' onclick="setTemp(0)" value="&#11015;"/>
<form id="FTEMP_" style="display:inline"><input name="TEMP" id="TEMP" type="text" value="_TEMP_" style="width:20px"/></form>
<input class="button" type='button' onclick="setTemp(1)" value="&#11014;"/>
</td>
</tr>
</table>
<center><form><input class="button" type='submit' name='CONNECT' value='Re-Connect'/></form></center>
</body>
</html>)====";
