<pre><?
	// The url that will be written to the version.xml file (to retrieve modules)
	$MODULESURL = 'http://www.hartte.de/uoclient/modules/';
	
	// This is the path to 7za which you can download (as "p7zip") from http://www.7-zip.org
	$ARCPATH = '/usr/bin/7za';

	// This little script is inteded to update the version.xml file using
	// the information from the modules directory.
		
	clearstatcache(); // Make sure we get the latest stat() information for files
	
	// Iterate trough the files in the modules directory
	$dp = opendir("modules");
	
	if (!$dp) {
		echo "Unable to open modules directory for reading.";
		exit();
	}
	
	// Check if a line is a marker line
	function isMarker($line) {
		for ($i = 0; $i < strlen($line); ++$i) {
			if ($line[$i] != ' ' && $line[$i] != "\n" && $line[$i] != '-') {
				return false;
			}			
		}
		return $line[0] == '-';
	}	
	
	$version_info = sprintf("<version generated=\"%u\">\n", time());
	
	// Read the entries in the directory
	while ($file = readdir($dp)) {
		// Don't process directories
		if (!is_file('modules/' . $file)) {
			continue;
		}
		
		// Don't process files that aren't 7zip archives
		$pos = strrpos(strtolower($file), ".7z");
		if ($pos === FALSE) {
			continue;
		}		
		
		// Dont't process unreadable files
		if (!is_readable('modules/' . $file)) {
			echo "WARNING: Unreadable file $file found.\n";
			continue;
		}
				
		echo "Processing file $file...\n";
		
		// List the files contained in here
		$command = $ARCPATH . ' l ' . escapeshellarg("modules/" . $file);
		$result = shell_exec($command);
		
		// Error?
		if (strpos($result, 'Error') !== FALSE) {
			echo "An error occured while reading the archive: $result.\n";
			continue;
		}
		
		$filesize = filesize('modules/' . $file);
		$version_info .= sprintf("\t<module size=\"%u\" name=\"%s\" url=\"%s\">\n", $filesize, $file, $MODULESURL . $file);
		
		$lines = explode("\n", $result);
		
		// Process line by line and find the start and end markers.
		$readingNames = false;
				
		for ($i = 0; $i < count($lines); ++$i) {
			$line = $lines[$i];
			
			// The markers are interpreted as toggles
			if (isMarker($line)) {
				$readingNames = !$readingNames;
				continue;
			}
			
			// We're reading files
			if ($readingNames) {
				// Split the line into the parts
				$parts = explode(' ', $line);
				
				// A typical line:
				// 2005-09-16 20:32:54 ....A      1867776       653097  python24.dll
				// 2005-09-18 13:23:54 ....A         7763               scripts/UserDict.pyo
				// We're not interested in the file size (although it would be an interesting extension)
				// So we use the first two and the last element
				$filename = trim($parts[count($parts)-1]);
				$date = trim($parts[0]);
				list($year, $month, $day) = explode('-', $date);
				$time = trim($parts[1]);
				list($hour, $minute, $second) = explode(':', $time);

				// Create a timestamp for further processing
				$timestamp = mktime($hour, $minute, $second, $month, $day, $year);
				
				// Correct the timezone to UTC
				$timestamp -= intval(date('Z'));
				
				// Debugging
				//echo "$filename: " . date('Y-m-d H:i:s', $timestamp) . "\n";
				
				$version_info .= "\t\t" . sprintf('<file path="%s" modified="%u" />', $filename, $timestamp) . "\n";
			}
		}
		
		$version_info .= "\t</module>\n\n";
	}
	
	$version_info .= "</version>\n";
	
	closedir($dp);
	
	$fp = fopen('version.xml', 'wt');
	fwrite($fp, $version_info);	
	fclose($fp);
?></pre>
<b>Generated version information:</b><br>
<pre><?
	echo
	htmlspecialchars($version_info)
?></pre>