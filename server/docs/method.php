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
<?
	require_once('header.inc.php');
?>
<div align="center">
  <p>      <span class="maintitle">Wolfpack Object Reference </span></p>
  <table width="780" border="0" cellspacing="3" cellpadding="0">
  <tr valign="top">
    <td class="text"><?
	$row = NULL;
	if ($_REQUEST['object'] && $_REQUEST['method']) {
		$object = mysql_escape_string($_REQUEST['object']);
		$method = mysql_escape_string($_REQUEST['method']);
		$result = mysql_query("SELECT `object`,`method`,`prototype`,`parameters`,`returnvalue`,`description` FROM documentation_objects_methods WHERE `object` = '$object' AND `method` = '$method';");
		if (mysql_num_rows($result) > 0) {
			$row = mysql_fetch_array($result);
			$count = mysql_num_rows($result);
		}
		mysql_free_result($result);
	}

	if ($row != NULL) {
	?><span class="sectiontitle"> METHOD DESCRIPTION</span><br>		  <br>
          <strong>Method Name:</strong>          <?=$row[1]?>
          <br>
          <hr size="1" noshade>
		  <?
		  	// There could be multiple methods
			$i = 0;
			$result = mysql_query("SELECT `object`,`method`,`prototype`,`parameters`,`returnvalue`,`description` FROM documentation_objects_methods WHERE `object` = '$object' AND `method` = '$method';");
			while ($row = mysql_fetch_array($result)) {
		  ?>          <code><?=$row[2]?>        </code><br>
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
          <strong>Return Value: </strong><br>
            <?=$row[4]?>        <p>
        <?
		  	}
		  	if (strlen($row[5]) > 0) {
		  ?>
        <strong>Description:</strong><br>
        <?=$row[5]?>
        <br>
        <?
		  	}
		  ?>
		  <hr size="1" noshade>
          <?
		  }
	 }

?>
</p>      </td>
    </tr>
  <tr valign="top">
    <td class="text"><table width="100%"  border="0" cellspacing="0" cellpadding="2">
      <tr>
        <td colspan="7"><span class="sectiontitle">OBJECT METHODS</span></td>
      </tr>
      <?
		$commands = array();
		$result = mysql_query("SELECT `method` FROM documentation_objects_methods ORDER BY `method` ASC;");
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
      <br></td>
    </tr>
  <tr valign="top">
    <td class="text"><a href="object.php?object=<?=$_REQUEST['object']?>">Back to the Object Details </a><br>
      <a href="object.php">Back to the Object Reference </a><a href="index.php"><br>
      Back to the Wolfpack Reference</a></td>
  </tr>
</table>
  <br>
  <?
  	require_once('footer.inc.php');
  ?></div>
</body>
</html>
