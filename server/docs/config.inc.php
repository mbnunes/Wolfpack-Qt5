<?
	// Change these settings to whatever you need.
	$db_host = 'localhost';
	$db_name = 'wpdev';
	$db_username = 'wpdev';
	$db_password = 'indiana';
	
	// Connect to the database host.
	if (!@mysql_connect($db_host, $db_username, $db_password)) {
		echo "Couldn't connect to the database host.";
		exit();
	}
	
	// Select database.
	if (!@mysql_select_db($db_name)) {
		echo "Couldn't select database.";
		exit();
	}
	
	// Close database connection when shutting down.
	function shutdown_handler() {
		// Close database connection.
		mysql_close();
	}
	
	// Register a cleanup handler.
	register_shutdown_function(shutdown_handler);
	
	$SETTINGS = array();	
	
	// Fetch all settings
	$result = mysql_query("SELECT `key`,`value` FROM documentation_settings;");
	while ($row = mysql_fetch_array($result)) {
		$SETTINGS[$row[0]] = $row[1];
	}
	mysql_free_result($result);
?>
