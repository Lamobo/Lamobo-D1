//选中/取消 自动获取IP
function checkAutoIp(a){
	if(a.checked){
		//$("#ip").val("");
		//$("#subnetMask").val("");
		//$("#gateWay").val("");
		//$("#dnsServer1").val("");
		//$("#dnsServer2").val("");
		$.each($("input[type='text']"),function(i,l){
			$(l).attr("disabled",true);
		})
	}else{
		$.each($("input[type='text']"),function(i,l){
			$(l).attr("disabled",false);
		})
	}
}

//在输入框按下 回车、点、右方向键
function keyEvent(a,e){
	a.value=a.value.replace(/\D/g,'');
	var key = window.event?e.keyCode:e.which;
	if(key==13||key==110||key==39){
		var ele=document.forms[0].elements;
		for(var i=0;i<ele.length;i++){   
			var q=(i==ele.length-1)?0:i+1;
			if(a==ele[i]){
				ele[q].focus();
				break;
			}   
		}
		var str = $(a).val();
		str = str.replace(".", "");
		$(a).val(str);
	}
}

//显示以太网配置输入框
function showNetworkConfigInput(a,event){
	var IPlikeInputStr=[]; 
	for(var i=0;i<4;i++){
		IPlikeInputStr[i]="<input type='text' onkeyup='keyEvent(this,event)' name='"+a+i+"' id='"+a+i+"' style='width:59px;padding:4px 0 0 0;' maxlength=3 value=''/>"+(i==3?"":"·");
	} 
	document.write("<div style='width:280px'>"+IPlikeInputStr.join("")+"</div>"); 
}

//提交以太网配置
function submitNetworkConfig(){
	if($("#autoIp").attr("checked")==false){
		var s=0;
		$.each($("input[type='text']"),function(i,l){
			var num = $(l).val();
			if(i<=19 && i>=16 && num!=""){
				s=1;
			}
			if(s==1){
				if((255>=parseInt(num) && 0<=parseInt(num))==false){
					alert("请正确填写备用DNS服务器，只能填写0-255之间的数字！");
					return false;
				}
			}
			if((255>=parseInt(num) && 0<=parseInt(num))||(i<=19 && i>=16)){
				if(i===$("input[type='text']").length-1){
					$("#ip").val($("#ip0").val()+"."+$("#ip1").val()+"."+$("#ip2").val()+"."+$("#ip3").val());
					$("#subnetMask").val($("#subnetMask0").val()+"."+$("#subnetMask1").val()+"."+$("#subnetMask2").val()+"."+$("#subnetMask3").val());
					$("#gateWay").val($("#gateWay0").val()+"."+$("#gateWay1").val()+"."+$("#gateWay2").val()+"."+$("#gateWay3").val());
					$("#dnsServer1").val($("#dnsServer10").val()+"."+$("#dnsServer11").val()+"."+$("#dnsServer12").val()+"."+$("#dnsServer13").val());
					$("#dnsServer2").val($("#dnsServer20").val()+"."+$("#dnsServer21").val()+"."+$("#dnsServer22").val()+"."+$("#dnsServer23").val());
					document.networkForm.submit();
				}
			}else{
				if(i>=0 && i<=3){
					alert("请正确填写IP，只能填写0-255之间的数字！");
				}
				if(i>=4 && i<=7){
					alert("请正确填写子网掩码，只能填写0-255之间的数字！");
				}
				if(i>=8 && i<=11){
					alert("请正确填写默认网关，只能填写0-255之间的数字！");
				}
				if(i>=12 && i<=15){
					alert("请正确填写首选DNS服务器，只能填写0-255之间的数字！");
				}
				
				return false;
			}
		})
	}else{
		document.networkForm.submit();
	}
}

//显示以太网配置数据
function setNetworkConfig(){
	var ip = $("#ip").val();
	var sm = $("#subnetMask").val();
	var gw = $("#gateWay").val();
	var fds = $("#dnsServer1").val();
	var lds = $("#dnsServer2").val();
	if(ip!=""){
		for(i=0;i<ip.split(".").length;i++){
			$("#ip"+i).val(ip.split(".")[i]);
		}
	}
	if(sm!=""){
		for(i=0;i<sm.split(".").length;i++){
			$("#subnetMask"+i).val(sm.split(".")[i]);
		}
	}
	
	if(gw!=""){
		for(i=0;i<gw.split(".").length;i++){
			$("#gateWay"+i).val(gw.split(".")[i]);
		}
	}
	
	if(fds!=""){
		for(i=0;i<fds.split(".").length;i++){
			$("#dnsServer1"+i).val(fds.split(".")[i]);
		}
	}
	
	if(lds!=""){
		for(i=0;i<lds.split(".").length;i++){
			$("#dnsServer2"+i).val(lds.split(".")[i]);
		}
	}
}

//选择wifi
function selectWifi(a){
	$("#ssid").val($($(a).children().first().next()).html());
	$("#wifiMode").val($($(a).children().first().next().next()).html());
	$("#safe").val($($(a).children().first().next().next().next()).html());
	$("#password").val("");
	if($("#safe").val()=='NONE'){
		$("#password").attr("disabled",true);
	}else{
		$("#password").attr("disabled",false);
	}
}


//选择wifi配置提交验证
function wifiConfigVerify(){
	if(document.wifiForm.elements["ssid"].value==""){
		alert("请在下方单击选择一个Wifi！");
		return false;
	}
	if(document.wifiForm.elements["safe"].value!="NONE" && document.wifiForm.elements["password"].value==""){
		alert("请填写密码！");
		return false;
	}
	return true;
}

// 显示添加wifi
function showAddWifi(){
	$("#selectWifiDiv").hide();
	$("#addWifiDiv").show();	
}

// 显示选择wifi
function showSelectWifi(){
	$("#addWifiDiv").hide();
	$("#selectWifiDiv").show();	
}

// 添加wifi选择加密类型
function changeWifiSafe(s){
	if(s=="NONE"){
		document.addWifiForm.elements["password"].disabled=true;
	}else{
		document.addWifiForm.elements["password"].disabled=false;
	}
}

//手工添加wifi配置提交验证
function addWifiConfigVerify(){
	if(document.addWifiForm.elements["ssid"].value==""){
		alert("请填写SSID！");
		return false;
	}
	if(document.addWifiForm.elements["safe"].value!="NONE" && document.addWifiForm.elements["password"].value==""){
		alert("请填写密码！");
		return false;
	}
	return true;
}


//用户管理提交验证
function userVerify(){
	var u = document.getElementById("userName").value;
	var p = document.getElementById("password").value;
	var p1 = document.getElementById("password2").value;
	if(u==null || u==""){
		alert("请填写用户名！");
		return false;
	}
	if(p==null || p==""){
		alert("请填写密码！");
		return false;
	}
	
	if(p1==null || p1==""){
		alert("请填写确认密码！");
		return false;
	}
	if(p==p1){
		return true;
	}else{
		alert("两次密码填写不一致！");
		return false;
	}
}

//恢复出厂设置提交验证
function confirmReset(){
	if (confirm("是否确认恢复出厂设置？")){ 
		return true; 
	}else{ 
		return false; 
	} 
}

//重启服务端确认
function confirmReboot() {
	if (confirm("是否确认重启服务端？")){ 
		return true; 
	}else{ 
		return false; 
	} 
}

//系统配置提交验证
function sysConfigVerify(){
	if (document.getElementById("password").value==""){ 
		alert("请填写访问密码！");
		return false; 
	}
	return true;
}



//码流2编码格式切换
function codedFormat2(a){
	if("mjpeg"==a){
		$("#staticRadioKpbsTwo").attr("disabled",true);
		$("#runRadioKpbsTwo").attr("disabled",true);
		$("#iFrameRateTwo").attr("disabled",true);
		$("#picQuality").attr("disabled",false);
		 
	}
	if("h264"==a){
		$("#staticRadioKpbsTwo").attr("disabled",false);
		$("#runRadioKpbsTwo").attr("disabled",false);
		$("#iFrameRateTwo").attr("disabled",false);
		$("#picQuality").attr("disabled",true);
	}
}

//视频设置提交检查
function videoConfigVerify(){

	if($("#staticKpbs").val()<256||$("#staticKpbs").val()>6144){
		alert("码流1码率填写错误！");
		return false;
	}
	
	if($("#staticKpbs2").val()<64||$("#staticKpbs2").val()>6144){
		alert("码流2码率填写错误！");
		return false;
	}
	
	if($("#frameRateOne").val()<1||$("#frameRateOne").val()>30){
		alert("码流1帧率填写错误！");
		return false;
	}
	if($("#iFrameRateOne").val()<2||$("#iFrameRateOne").val()>150){
		alert("码流1Ｉ帧率间隔填写错误！");
		return false;
	}
	if($("#frameRateTwo").val()<1||$("#frameRateTwo").val()>30){
		alert("码流2帧率填写错误！");
		return false;
	}
	if ($("#frameRateTwo").val()!=$("#frameRateOne").val()){
		alert("码流1和2帧率不相等！请设置成相等的帧率");
		return false;	
	}
	if($("#staticRadioKpbsTwo").attr("disabled")==false){
		if($("#iFrameRateTwo").val()<2||$("#iFrameRateTwo").val()>150){
			alert("码流2Ｉ帧率间隔填写错误！");
			return false;
		}
	}
	return true;
}


// 视频设置界面初始化处理
function videoConfigInit(){
	
	codedFormat2($("#codedFormatTwo").val());
	
}


//softAP提交验证
function softAPCheckUp(){
	var s = document.getElementById("ssid").value;
	var p = document.getElementById("psd").value;
	var p1 = document.getElementById("confirmPassword").value;
	if(""==s){
		alert("SSID不能为空！");
		return false;
	}
	if (s.length > 32){
   		alert("SSID不能超过32个字符，请重新设置！");
  		return false;
	}
   	if(""==p){
		alert("请填写密码！");
		return false;
	}
	if(p.length<8 || p.length>63){
		alert("请填写正确的密码，密码不能小于8位且不能大于63位！");
		return false;
	}
	if(p!=p1){
		alert("两次填写的密码不一致！");
		return false;
	}
	return true;
}

//应用提交验证
function appCheckUp(){
	var a = document.getElementById("updateUrl").value;
	if(""==a){
		alert("请填写应用升级地址！");
		return false;
	}
	$("#appChar").css("display","none");
	$("#updateUrl").css("display","none");
	$("#appUpdate").css("display","block");
	$(":submit").css("display","none");
	return true;
}

//内核提交验证
function rootCheckUp(){
	var r = document.getElementById("rootUrl").value;
	if(""==r){
		alert("请填写内核升级地址！");
		return false;
	}
	$("#rootChar").css("display","none");
	$("#rootUrl").css("display","none");
	$("#rootUpdate").css("display","block");
	$(":submit").css("display","none");
	return true;
}
	
//登录验证
function loginVerify(){
	var u = document.getElementById("userName").value;
	var p = document.getElementById("password").value;
	if(u==null || u==""){
		alert("请填写用户名！");
		return false;
	}
	if(p==null || p==""){
		alert("请填写密码！");
		return false;
	}
	return true;
}

function refreshPage(){
	window.location.href=window.location.href;
}

