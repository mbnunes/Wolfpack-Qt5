<?php require_once('config.inc.php'); ?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
	<head>
	<title>Wolfpack Reference</title>
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
		<link href="./style.css" rel="stylesheet" type="text/css" />
	</head>
	<body>
<?php require_once('header.inc.php'); ?>
		<a name="top"></a>
		<div align="center">
			<p><span class="maintitle">Wolfpack Object Reference</span></p>
			<table width="780" border="0" cellspacing="3" cellpadding="0">
			<tr valign="top">
				<td class="text">
<?php
	$row = NULL;
	if ( $_REQUEST['object'] )
	{
		$name = mysql_escape_string( $_REQUEST['object'] );
		$result = mysql_query("SELECT `object`,`description` FROM `documentation_objects` WHERE `object` = '$name';");
		if ( mysql_num_rows( $result ) > 0 )
		{
			$row = mysql_fetch_array( $result );
		}
		mysql_free_result( $result );
	}

	if ( $row || $row != NULL )
	{
?>
					<p><span class="sectiontitle">OBJECT DESCRIPTION</span><br /><br />
					<strong>Name:</strong>&#160;<?=$row[0]?><br /><br />
<?php
		if ( strlen( $row[1] ) > 0 )
		{
?>
					<strong>Description:</strong><br /><?=$row[1]?><br />
<?php
		}
?>
					<br />
					<strong>Methods:</strong>
					<table width="100%"  border="0" cellspacing="0" cellpadding="2">
<?php
		$commands = array();

		$result = mysql_query( "SELECT `method` FROM `documentation_objects_methods` WHERE `object` = '$name' ORDER BY `method` ASC;" );
		while ( $row = mysql_fetch_array( $result ) )
		{
			array_push( $commands, $row[0] );
		}
		mysql_free_result( $result );

		$cols = 7;
		$rows = ceil( sizeof( $commands ) / $cols );

		for ( $row = 0; $row < $rows; ++$row )
		{
			echo "<tr>\n";
			for ( $col = 0; $col < $cols; ++$col )
			{
				$id = $col * $rows + $row;
				if ( $id < sizeof( $commands ) )
				{
?>
							<td>-&#160;<a href="#meth_<?=$commands[$id]?>"><?=$commands[$id]?></a></td>
<?php
				}
				else
				{
					echo "<td>&#160;</td>\n";
				}
			}
			echo "</tr>\n";
		}
?>
					</table>
					<p>
						<strong>Properties:</strong>
						<table width="100%"  border="0" cellspacing="0" cellpadding="2">
<?php
		$commands = array();

		$result = mysql_query( "SELECT `property` FROM `documentation_objects_properties` WHERE `object` = '$name' ORDER BY `property` ASC;" );
		while ( $row = mysql_fetch_array( $result ) )
		{
			array_push( $commands, $row[0] );
		}
		mysql_free_result( $result );

		$cols = 7;
		$rows = ceil( sizeof( $commands ) / $cols );

		for ( $row = 0; $row < $rows; ++$row )
		{
			echo "<tr>\n";
			for ( $col = 0; $col < $cols; ++$col )
			{
				$id = $col * $rows + $row;
				if ( $id < sizeof( $commands ) )
				{
?>
								<td>-&#160;<a href="#prop_<?=$commands[$id]?>"><?=$commands[$id]?></a></td>
<?php
				}
				else
				{
					echo "<td>&#160;</td>\n";
				}
			}
			echo "</tr>\n";
		}
?>
						</table>
						<p>
						<span class="sectiontitle">OBJECT METHODS</span><br />
						<br />
<?php
		// There could be multiple methods
		$i = 0;
		$result = mysql_query("SELECT `object`,`method`,`prototype`,`parameters`,`returnvalue`,`description` FROM `documentation_objects_methods` WHERE `object` = '$name' ORDER BY `method` ASC;");
		$count = mysql_num_rows($result);
		while ($row = mysql_fetch_array($result))
		{
?>
						<a name="meth_<?=$row[1]?>"></a>
						<b><code style="font-size: 12px"><?=$row[2]?></code></b><br />
						<br />
<?php
			if ( strlen($row[3]) > 0 ) {
?>
						<?=$row[3]?><br /><br />
<?php
			}
			if ( strlen( $row[4] ) > 0 )
			{
?>
						<span class="style2">Return Value:</span><br /><?=$row[4]?><br />
<?php
			}
			if ( strlen( $row[5] ) > 0 )
			{
?>
						<span class="style2">Description:</span><br />
						<?=$row[5]?><br />
<?php
			}
?>
						<br /><a href="#top">Back to top</a>
<?php
			if ( ++$i < $count )
			{
				echo '<hr size="1" noshade>';
			}
		}
?>
						<span class="sectiontitle"><br /><br />OBJECT PROPERTIES</span><br /><br />
<?php
		// There could be multiple methods
		$i = 0;
		$result = mysql_query("SELECT `property`,`description`,`readonly` FROM `documentation_objects_properties` WHERE `object` = '$name' ORDER BY `property` ASC;");
		$count = mysql_num_rows($result);
		while ($row = mysql_fetch_array($result))
		{
?>
						<a name="prop_<?=$row[0]?>"></a><b><code style="font-size: 12px"><?=$row[0]?></code></b><? if ($row[2] == 1) { echo '(read only)'; } ?><br /><br />
<?php
			if ( strlen($row[1]) > 0 )
			{
?>
						<span class="style2">Description:</span><br />
						<?=$row[1]?><br />
<?php
		  	}
?>
						<br /><a href="#top">Back to top</a>
<?php
		    if ( ++$i < $count )
			{
				echo '<hr size="1" noshade>';
			}
		}
?>
	<br /><br />
<?php
	}
	else
	{
?>
						<span class="sectiontitle">SCRIPTING OBJECTS</span><br />
						An object in Wolfpack has several methods (functions working with the object) and property (data 	assigned to the object). This section will serve as a reference to all available classes of objects in 	Wolfpack. <br />
						Choose one of the object classes from the list at the bottom.
					</p>
					<table width="100%"  border="0" cellspacing="0" cellpadding="2">
<?php
		$commands = array();
		$result = mysql_query("SELECT `object` FROM `documentation_objects` ORDER BY `object` ASC;");
		while ( $row = mysql_fetch_array( $result ) )
		{
			array_push( $commands, $row[0] );
		}
		mysql_free_result( $result );

		$cols = 7;
		$rows = ceil( sizeof( $commands ) / $cols );

		for ( $row = 0; $row < $rows; ++$row )
		{
			echo "<tr>\n";
			for ( $col = 0; $col < $cols; ++$col )
			{
				$id = $col * $rows + $row;
				if ( $id < sizeof( $commands ) )
				{
?>
							<td>-&#160;<a href="object.php?object=<?=$commands[$id]?>"><?=$commands[$id]?></a></td>
<?php
				}
				else
				{
					echo "<td>&#160;</td>\n";
				}
			}
			echo "</tr>\n";
		}
?>
					</table>
					<br />
<?php
	}
?>
				</td>
				</tr>
				<tr valign="top">
					<td class="text">
						<a href="index.php"></a><a href="object.php">Back to the Object Overview </a><a href="index.php"><br />
						Back to the Wolfpack Reference</a>
					</td>
				</tr>
			</table>
			<br />
<?php require_once('footer.inc.php'); ?>
		</div>
	</body>
</html>
