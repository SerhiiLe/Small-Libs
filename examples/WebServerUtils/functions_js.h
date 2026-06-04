#ifndef functions_js_h
#define functions_js_h

// Script that fills out a form
const char FUNCTIONS_JS[] PROGMEM = R"rawtext(
function ajaxRequest(url, aMethod, param, onSuccess, onFailure) {
	var aR = new XMLHttpRequest();
	aR.onreadystatechange = function() {
		if( aR.readyState == 4 && (aR.status == 200 || aR.status == 304))
			onSuccess(aR);
		else if (aR.readyState == 4 && aR.status != 200) onFailure(aR);
	};
	aR.open(aMethod, url, true);
	if (aMethod == 'POST') aR.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded; charset=utf-8');
	aR.send(param);
};
function dummy() {return;}
function fill_settings(url,form_name,cbfunc=null) {
	ajaxRequest(url,"GET",null, function(ajaxResp) {
		var doc = JSON.parse(ajaxResp.responseText);
		var f = document.forms[form_name];
		for (var key in doc) {
			if(!f.elements[key]) continue;
			if(f.elements[key].type=="checkbox") {
				if( f.elements[key].checked && doc[key] == 0 )
					f.elements[key].checked = false;
				if( ! f.elements[key].checked && doc[key] != 0 )
					f.elements[key].checked = true;
			} else if(f.elements[key].type=="time") {
				var h = Math.floor(doc[key]/60);
				var m = doc[key]%60;
				f.elements[key].value = (h<10?"0"+h:h) + ":" + (m<10?"0"+m:m);
			} else
				f.elements[key].value = doc[key];
		}
		if(cbfunc !== null) cbfunc();
	}, dummy);
};
)rawtext";

#endif