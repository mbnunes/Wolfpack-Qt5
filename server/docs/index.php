<?
	require_once('config.inc.php');
	global $SETTINGS;
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
<div align="center"><br><span class="maintitle">Wolfpack Reference</span><br>
<br>
<table width="740" border="0" cellspacing="3" cellpadding="2">
  <tr align="left" valign="top">
    <td width="33%" class="text"><div align="left"><span class="sectiontitle">INGAME COMMANDS</span><br> 
          These commands can be used by players, gms and admins while in the game. This section gives you an overview over all available commands and how to use them.<br>
          <a href="command.php">Read more...  (<?
		  	$result = mysql_query("SELECT COUNT(*) FROM documentation_commands;");
			echo array_pop(mysql_fetch_array($result));
			mysql_free_result($result);
		  ?> commands)</a><br>      
          <br>
    </div></td>
    <td width="33%" class="text"><span class="sectiontitle">SCRIPTING OBJECTS</span><br>
      This section of the documentation gives you an overview over available wolfpack specific objects, their properties and methods and in some cases how to create them.<br>
      <a href="#">Read more...</a> </td>
    <td width="33%" class="text"><span class="sectiontitle">SCRIPTING LIBRARY</span><br>
      Wolfpack comes with a sophisticated library of scripts. This section of the documentation provides an overview over the library and its functions.<br>
      <a href="#">Read more... </a></td>
  </tr>
</table> 
<hr width="740" size="1" noshade>

<table width="740" border="0" cellpadding="2" cellspacing="3">
  <tr align="left" valign="top">
    <td width="33%" class="text"><span class="sectiontitle">SCRIPTING EVENTS</span><br>
      This section describes the events your scripts will be notified about.<br>
      <a href="event.php">Read more... (<?
		  	$result = mysql_query("SELECT COUNT(*) FROM documentation_events;");
			echo array_pop(mysql_fetch_array($result));
			mysql_free_result($result);
		  ?> events)</a><br></td><td width="33%" class="text"><span class="sectiontitle">DEFINITION TAGS</span><br>
        This section provides an overview over the tags used in the Wolfpack XML definitions. 
        <a href="#">Read more...</a></td>
    <td width="33%" class="text"><span class="sectiontitle">MISCELLANEOUS</span><br>
      Miscelleneous information can be found in this section. Links, configuration and setup among other things.<br>
      <a href="#">Read more...</a> </td>
  </tr>
</table>
<br>
<?
	require_once('footer.inc.php');
?></div>
</body>
</html>
