<?
	require_once('config.inc.php');
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<title>Wolfpack Reference</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<style type="text/css">
<!--
body {
	margin-left: 0px;
	margin-top: 0px;
	margin-right: 0px;
	margin-bottom: 0px;
}
-->
</style>
<link href="style.css" rel="stylesheet" type="text/css">
<style type="text/css">
<!--
a:link {
	color: #0000FF;
	text-decoration: none;
}
a:visited {
	text-decoration: none;
	color: #0000FF;
}
a:hover {
	text-decoration: underline;
}
a:active {
	text-decoration: none;
}
-->
</style></head>

<body>
<table width="100%"  border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td bgcolor="#004B2C"><img src="top_logo.jpg" width="586" height="87"></td>
  </tr>
  <tr>
    <td height="3" bgcolor="#999999"><img src="spacer.gif" width="1" height="3"></td>
  </tr>
</table>
<div align="center">
  <p>      <span class="maintitle">Wolfpack Command Reference </span></p>
  <table width="780" border="0" cellspacing="3" cellpadding="0">
  <tr valign="top">
    <td width="81%" class="text"><?
	$row = NULL;
	if ($_REQUEST['name']) {
		$name = mysql_escape_string($_REQUEST['name']);
		$result = mysql_query("SELECT `name`,`description`,`usage`,`notes` FROM documentation_commands WHERE `name` = '$name';");
		if (mysql_num_rows($result) > 0) {
			$row = mysql_fetch_array($result);
		}
		mysql_free_result($result);
	}
	
	if ($row != NULL) {
	?><span class="sectiontitle">COMMAND DESCRIPTION</span><br>
        <br>        <strong>Command Name:</strong> <?=$row[0]?><br>
<br>
<strong>Short Description:</strong> <?=$row[1]?><br>
<br>
	<?
	if (strlen($row[2]) > 0) {
	?>
	<strong>Usage:</strong><br> 
	<?=$row[2]?><br>
	<br>
	<?
	}
	
	if (strlen($row[3]) > 0) {
	?>	
	<strong>Notes:</strong><br>
	<?=$row[3]?><br>
	<br>
<?
	}
}  else {
?>

<span class="sectiontitle">INGAME COMMANDS</span><br> 
To use one of the commands in this section, you have to know about your <em>Command Prefix</em> first. The default command prefix is the character <strong>'</strong>, but you can change it to any character you like in your wolfpack.xml file. If you know your command prefix, simply prepend it to the name of the command and say it ingame. The command will then be executed. <br> 
The name of the command is case insensitive.You can see a list of all available commands to the right.<br><br><?
}
?>
<a href="index.php">Back to the Wolfpack Reference </a></td>
    <td width="19%" class="text"><span class="sectiontitle">OTHER COMMANDS</span><br>
<?
	$result = mysql_query("SELECT name FROM documentation_commands ORDER BY name ASC;");
	while ($row = mysql_fetch_array($result)) {
?>
- <a href="command.php?name=<?=$row[0]?>"><?=$row[0]?></a><br>
<?
	}
	mysql_free_result($result);
?></td>
  </tr>
</table> 
  <br>
  <?
  	require_once('footer.inc.php');
  ?></div>
</body>
</html>
