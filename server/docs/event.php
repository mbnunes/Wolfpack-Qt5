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
  <p>      <span class="maintitle">Wolfpack Event Reference </span></p>
  <table width="780" border="0" cellspacing="3" cellpadding="0">
  <tr valign="top">
    <td class="text"><?
	$row = NULL;
	if ($_REQUEST['name']) {
		$name = mysql_escape_string($_REQUEST['name']);
		$result = mysql_query("SELECT `name`,`prototype`,`parameters`,`returnvalue`,`callcondition`,`notes` FROM documentation_events WHERE `name` = '$name';");
		if (mysql_num_rows($result) > 0) {
			$row = mysql_fetch_array($result);
		}
		mysql_free_result($result);
	}
	
	if ($row != NULL) {
	?>
      <p><span class="sectiontitle">EVENT DESCRIPTION</span><br>
          <br>        
          <strong>Event Name:</strong> <?=$row[0]?><br>
          <br>
          <strong>Prototype:</strong><br>
          <?=$row[1]?><br>
          <br>
		  <?
		  	if (strlen($row[2]) > 0) {
		  ?>
          <strong>Parameters:</strong><br>        
<?=$row[2]?><br>
<br>
<?
	}
	if (strlen($row[3]) > 0) { ?>
<strong>Return Value: </strong><br>
<?=$row[3]?><br>
          <br>
		  <?
		  	}
		  	if (strlen($row[4]) > 0) {
		  ?>
          <strong>Call Condition:</strong><br>
		  <?=$row[4]?><br>
          <br>
		  <?
		  	}
		  	if(strlen($row[5]) > 0) {
		  ?>
          <strong>Notes:</strong><br>
          <?=$row[5]?><br> 
          <br>
          <?
		  }
	}  else {
?>
          <span class="sectiontitle">SCRIPTING EVENTS</span><br>
          Your scripts will be notified of certain events by event functions. To use one of these events, simply place the event function in your script. The name of event functions are case sensitive.<br>
          You can see a list of all available events at the bottom of this page.<br>
      </p>
      <?
}
?></td>
    </tr>
  <tr valign="top">
    <td class="text"><span class="sectiontitle">OTHER EVENTS </span><br>
      <table width="100%"  border="0" cellspacing="0" cellpadding="2">
      <?
		$commands = array();
		$result = mysql_query("SELECT name FROM documentation_events ORDER BY name ASC;");
		while ($row = mysql_fetch_array($result)) {	
			array_push($commands, $row[0]);
		}
		mysql_free_result($result);
	  
		$cols = 7;
		$rows = ceil(sizeof($commands) / $cols);

		for ($row = 0; $row < $rows; ++$row) {		
			echo "<tr>\n";		
			for ($col = 0; $col < $cols; ++$col) {										
				$id = $col * $rows + $row;
				if ($id < sizeof($commands)) {
?>
	<td>- <a href="event.php?name=<?=$commands[$id]?>"><?=$commands[$id]?></a></td>
<?
				} else {
					echo "<td>&nbsp;</td>\n";
				}
			}
			echo "</tr>\n";
		}
?>
	</table><br>	</td>
    </tr>
  <tr valign="top">
    <td class="text"><a href="index.php">Back to the Wolfpack Reference</a></td>
  </tr>
</table> 
  <br>
  <?
  	require_once('footer.inc.php');
  ?></div>
</body>
</html>
