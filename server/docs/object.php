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
  <p>      <span class="maintitle">Wolfpack Object Reference </span></p>
  <table width="780" border="0" cellspacing="3" cellpadding="0">
  <tr valign="top">
    <td class="text"><?
	$row = NULL;
	if ($_REQUEST['object']) {
		$name = mysql_escape_string($_REQUEST['object']);
		$result = mysql_query("SELECT `object`,`description` FROM documentation_objects WHERE `object` = '$object';");
		if (mysql_num_rows($result) > 0) {
			$row = mysql_fetch_array($result);
		}
		mysql_free_result($result);
	}	

	if ($row != NULL) {
	?>      <p><span class="sectiontitle">OBJECT DESCRIPTION</span><br>
      <br>
          <strong>Name:</strong>          <?=$row[0]?>        
          <br>        
          <br>
          <?
		  	if (strlen($row[1]) > 0) {
		  ?>
        <strong>Description:</strong><br>
        <?=$row[1]?>
        <br>
		<?
			}
		?>
        <table width="100%"  border="0" cellspacing="0" cellpadding="0">
		<tr>
			<td colspan="7"><span class="sectiontitle">OBJECT METHODS</span></td>
		</tr>
        <?
		$commands = array();
		$result = mysql_query("SELECT `method` FROM documentation_objects_methods WHERE `object`= '$row[0]' ORDER BY `method` ASC;");
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
  <td>- <a href="method.php?object=<?=$_REQUEST['object']?>&method=<?=$commands[$id]?>">
      <?=$commands[$id]?>
    </a></td>
      <?
				} else {
					echo "<td>&nbsp;</td>\n";
				}
			}
			echo "</tr>\n";
		}
?>
      </table>
	  <span class="sectiontitle"><br>
	  OBJECT PROPERTIES </span><br>          
		Nice little object properties<br>		<br>        
          <?
	}  else {
?>
          <span class="sectiontitle">SCRIPTING OBJECTS</span><br>
          An object in Wolfpack has several methods (functions working with the object) and property (data assigned to the object). This section will serve as a reference to all available classes of objects in Wolfpack. <br>
          Choose one of the object classes from the list at the bottom.      </p>      <table width="100%"  border="0" cellspacing="0" cellpadding="2">
        <?
		$commands = array();
		$result = mysql_query("SELECT `object` FROM documentation_objects ORDER BY `object` ASC;");
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
  <td>- <a href="object.php?object=<?=$commands[$id]?>">
      <?=$commands[$id]?>
    </a></td>
      <?
				} else {
					echo "<td>&nbsp;</td>\n";
				}
			}
			echo "</tr>\n";
		}
?>
      </table>
      <br>        <?
}
?></td>
    </tr>
  <tr valign="top">
    <td class="text"><a href="index.php"></a><a href="object.php">Back to the Object Overview </a><a href="index.php"><br>
      Back to the Wolfpack Reference</a></td>
  </tr>
</table> 
  <br>
  <?
  	require_once('footer.inc.php');
  ?></div>
</body>
</html>
