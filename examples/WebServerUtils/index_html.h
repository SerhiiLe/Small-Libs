#ifndef index_html_h
#define index_html_h

// Page code with form
const char INDEX_HTML[] PROGMEM = R"rawtext(
<!DOCTYPE html>
<html>
<head>
	<meta charset='UTF-8'>
	<title>ESP Web Server</title>
	<script src="functions.js" type="text/javascript" encoding="UTF-8"></script>
</head>
<body>
	<h2>Form on ESP</h2>
	<form name="myData" action='/submit' method='POST'>
		<label>String 'String':</label>
		<input type='text' name='text1' placeholder='Text1'>
		<br>
		<label>String 'char[]':</label>
		<input type='text' name='text2' placeholder='Text2'>
		<br>
		<label>Number:</label>
		<input type='number' name='number'>
		<br>
		<label>Select 'String':</label>
		<select name='select1'>
			<option>First</option>
			<option>Second</option>
			<option>Third</option>
		</select>
		<br>
		<label>Select 'Int':</label>
		<select name='select2'>
			<option value=0>First</option>
			<option value=1>Second</option>
			<option value=2>Third</option>
		</select>
		<br>
		<label>Checkbox:</label>
		<input type='checkbox' name='checkbox' value=1>
		<br>
		<label>Time:</label>
		<input type='time' name='time' value="00:00">
		<br>
		<label>Float:</label>
		<input type='text' name='point' value="0.0">
		<br>
		<label>Changed:</label>
		<input type='text' name='changed' value="" disabled>
		<br>
		<input type='submit' value='Send'>
		<a href="/data.json">show json data</a>
		<a href="/rewrite_files">Rewrite files</a>
	</form>
</body>
<script type="text/javascript" encoding="UTF-8">
function start() {
	if (typeof ajaxRequest !== "undefined") {
		fill_settings("data.json","myData");
	} else setTimeout(start, 100);
};
start();
</script>
</html>
)rawtext";

#endif