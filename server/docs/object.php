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
.style2 {
	color: #999999;
	font-weight: bold;
}
-->
</style></head>

<body>
<?
	require_once('header.inc.php');
?>
<a name="top"></a>
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
        <br>
        <strong>Methods:</strong>
      <table width="100%"  border="0" cellspacing="0" cellpadding="2">
        <?
		$commands = array();	
		
		$result = mysql_query("SELECT `method` FROM documentation_objects_methods WHERE `object` = '$name' ORDER BY `method` ASC;");
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
  <td>- <a href="#meth_<?=$commands[$id]?>">
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
      <p>          <strong>Properties:</strong>
      <table width="100%"  border="0" cellspacing="0" cellpadding="2">
        <?
		$commands = array();	
		
		$result = mysql_query("SELECT `property` FROM documentation_objects_properties WHERE `object` = '$name' ORDER BY `property` ASC;");
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
  <td>- <a href="#prop_<?=$commands[$id]?>">
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
      <p><span class="sectiontitle">OBJECT METHODS</span><br>
	      <br>
          <?
		  	// There could be multiple methods
			$i = 0;
			$result = mysql_query("SELECT `object`,`method`,`prototype`,`parameters`,`returnvalue`,`description` FROM documentation_objects_methods WHERE `object` = '$object' ORDER BY `method` ASC;");
			$count = mysql_num_rows($result);
			while ($row = mysql_fetch_array($result)) {
		  ?>
          <a name="meth_<?=$row[1]?>"></a>
          <b><code style="font-size: 12px">
          <?=$row[2]?>
                    </code></b><br>
          <br>
          <?
		  	if (strlen($row[3]) > 0) {
		  ?>
          <?=$row[3]?>
          <br>
          <br>
          <?
	}
	if (strlen($row[4]) > 0) { ?>
          <span class="style2">Return Value: </span><br>
          <?=$row[4]?>
        <p>
          <?
		  	}
		  	if (strlen($row[5]) > 0) {
		  ?>
          <span class="style2">Description:</span><br>
          <?=$row[5]?>
          <br>
          <?
		  	}
			?>
			<br><a href="#top">Back to top</a>
			<?

		    if (++$i < $count) {
        	  echo '<hr size="1" noshade>';
			}
		
       }

?>        <span class="sectiontitle"><br>
			<br>
			OBJECT PROPERTIES </span><br>
            <br>
            <?
		  	// There could be multiple methods
			$i = 0;
			$result = mysql_query("SELECT `property`,`description`,`readonly` FROM documentation_objects_properties WHERE `object` = '$object' ORDER BY `property` ASC;");
			$count = mysql_num_rows($result);
			while ($row = mysql_fetch_array($result)) {
		  ?>
            <a name="prop_<?=$row[0]?>"></a> <b><code style="font-size: 12px">
            <?=$row[0]?>
            </code></b><? if ($row[2] == 1) { echo '(read only)'; } ?><br>
            <br>
            <?
		  	if (strlen($row[1]) > 0) {
		  ?>
          <span class="style2">Description:</span><br>
          <?=$row[1]?>
          <br>
          <?
		  	}
			?>
          <br><a href="#top">Back to top</a>
          <?

		    if (++$i < $count) {
        	  echo '<hr size="1" noshade>';
			}
		
       }
?>
          <br>
          <br>        
          <?
	}  else {
?>
          <span class="sectiontitle">SCRIPTING OBJECTS</span><br>
          An object in Wolfpack has several methods (functions working with the object) and property (data assigned to the object). This section will serve as a reference to all available classes of objects in Wolfpack. <br>
        Choose one of the object classes from the list at the bottom.      </p>      
        <table width="100%"  border="0" cellspacing="0" cellpadding="2">
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
