<?php require_once('config.inc.php'); ?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
	<head>
		<title>Wolfpack Reference</title>
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
		<link href="./style.css" rel="stylesheet" type="text/css" />
	</head>
	<body>
<?php require_once('./header.inc.php'); ?>
		<div align="center">
			<p><span class="maintitle">Wolfpack Command Reference </span></p>
			<table width="780" border="0" cellspacing="3" cellpadding="0">
				<tr valign="top">
					<td class="text">
<?php
	$row = NULL;
	if ( $_REQUEST['name'] )
	{
		$name = mysql_escape_string( $_REQUEST['name'] );
		$result = mysql_query( "SELECT `name`,`description`,`usage`,`notes` FROM documentation_commands WHERE `name` = '$name';" );
		if ( mysql_num_rows( $result ) > 0 )
		{
			$row = mysql_fetch_array( $result );
		}
		mysql_free_result( $result );
	}
	if ( $row != NULL )
	{
?>
						<span class="sectiontitle">COMMAND DESCRIPTION</span><br />
						<br /><strong>Command Name:</strong>&#160;<?=$row[0]?><br />
						<br />
						<strong>Short Description:</strong>&#160;<?=$row[1]?><br />
						<br />
<?php
		if ( strlen( $row[2] ) > 0 )
		{
?>
						<strong>Usage:</strong><br />
						<?=$row[2]?><br />
						<br />
<?php
		}
		if ( strlen( $row[3] ) > 0 )
		{
?>
						<strong>Notes:</strong><br />
						<?=$row[3]?><br />
						<br />
<?php
		}
	}
	else
	{
?>
						<span class="sectiontitle">INGAME COMMANDS</span><br />
						To use one of the commands in this section, you have to know about your <em>Command Prefix</em> first. The default command prefix is the character <strong>'</strong>, but you can change it to any character you like in your wolfpack.xml file. If you know your command prefix, simply prepend it to the name of the command and say it ingame. The command will then be executed. <br />
						The name of the command is case insensitive.<br />
						You can see a list of all available commands at the bottom of this page.<br /><br />
<?php
	}
?>
					</td>
				</tr>
				<tr valign="top">
					<td class="text">
						<span class="sectiontitle">OTHER COMMANDS</span><br />
						<table width="100%"  border="0" cellspacing="0" cellpadding="2">
<?php
	$commands = array();
	$result = mysql_query( "SELECT name FROM documentation_commands ORDER BY name ASC;" );
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
							<td>-&#160;<a href="command.php?name=<?=$commands[$id]?>"><?=$commands[$id]?></a></td>
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
					</td>
				</tr>
				<tr valign="top">
					<td class="text"><a href="index.php">Back to the Wolfpack Reference</a></td>
				</tr>
			</table>
			<br />
<?php require_once('./footer.inc.php'); ?>
		</div>
	</body>
</html>
