#!/bin/sh
CONFIG_FILE="setup.cf";
SETUP_BASEDIR="setup.d";
NECESSARY_COMMANDS="bash seq run-parts expr grep sed cut head tail chmod";

#
# old sh didn't support functions. so don't use them at first!
#

echo -e "\e[1;32mChecking for base dependencies:\e[m";

# check for base commands {{{
	echo -e "\e[33m--> \e[1mAre necessary commands available?\e[m";
		echo -en "\e[36m---->\e[m ";
		for DEP in $NECESSARY_COMMANDS;
		do
			if [ ! "$(which $DEP)" ];
			then
				ERRORS="$ERRORS${ERRORS:+\n}\e[41m--EE> \e[1m\e[33m$DEP\e[m\e[1;41m is not in PATH!\e[m";
			elif [ ! -x "$(which $DEP)" ];
			then
				ERRORS="$ERRORS${ERRORS:+\n}\e[41m--EE> \e[1m\e[33m$DEP\e[m\e[1;41m is not executable!\e[m";
			fi;
			echo -en "\e[1;36m$DEP\e[m ";
		done;
		echo -e '\b\e[36m.\e[m';
		if [ "$ERRORS" ];
		then
			echo -e "$ERRORS" >&2;
			echo -e "\e[1;31mPlease correct the errors above and run again.\e[m" >&2;
			exit 1;
		fi;
	#
# }}}

# check for permissions which are required by run-parts {{{
	#
	# correct path if necessary
	#
	if ! echo "$SETUP_BASEDIR"|grep '^/' > /dev/null 2>&1;
	then
		SETUP_BASEDIR="$PWD/$SETUP_BASEDIR";
	fi
	SETUP_BASEDIR="$(echo -n "$SETUP_BASEDIR"|sed -e 's|/\.*/|/|g')";

	echo -e "\e[33m--> \e[1mPermissions correctly set?\e[m";
	#
	# does setup.d exist?
	#
		if [ -e "$SETUP_BASEDIR" ];
		then
			echo -e "\e[36m----> \e[1;35m$SETUP_BASEDIR\e[36m exists.\e[m";
		else
			echo -e "\e[41m--EE> \e[1m\e[33m$SETUP_BASEDIR\e[m\e[1;41m does not exist.\e[m" >&2;
			echo -e "\e[1;31mPlease correct the errors above and run again.\e[m" >&2;
			exit 1;
		fi;
	#
	# is it a directory?
	#
		if [ -d "$SETUP_BASEDIR" ];
		then
			echo -e "\e[36m----> \e[1;35m$SETUP_BASEDIR\e[36m is a directory.\e[m";
		else
			echo -e "\e[41m--EE> \e[1m\e[33m$SETUP_BASEDIR\e[m\e[1;41m is NOT a directory.\e[m" >&2;
			echo -e "\e[1;31mPlease correct the errors above and run again.\e[m" >&2;
			exit 1;
		fi;
	#
	# is the directory readable?
	#
		if [ -r "$SETUP_BASEDIR" ];
		then
			echo -e "\e[36m----> \e[1;35m$SETUP_BASEDIR\e[36m is readable.\e[m";
		else
			echo -e "\e[31m--WW> \e[1;35m$SETUP_BASEDIR\e[31m is not readable.\e[m";
			#
			# chmod +r if it's possible.
			#
				echo -e "\e[36m------> \e[1mSetting +r for \e[35m$SETUP_BASEDIR\e[36m.\e[m";
				if chmod +r "$SETUP_BASEDIR" > /dev/null 2>&1;
				then
					echo -e "\e[36m------> \e[1mSuccessfully made \e[35m$SETUP_BASEDIR\e[36m readable.\e[m";
				else
					echo -e "\e[41m----EE> \e[1mCould not chmod +r on \e[33m$SETUP_BASEDIR\e[m\e[1;41m.\e[m" >&2;
					echo -e "\e[1;31mPlease correct the errors above and run again.\e[m" >&2;
					exit 1;
				fi;
			#
		fi;
	#
	# is the directory executable?
	#
		if [ -x "$SETUP_BASEDIR" ];
		then
			echo -e "\e[36m----> \e[1;35m$SETUP_BASEDIR\e[36m is executable.\e[m";
		else
			echo -e "\e[31m--WW> \e[1;35m$SETUP_BASEDIR\e[31m is not executable.\e[m" >&2;
			#
			# chmod +x if it's possible.
			#
				echo -e "\e[36m------> \e[1mSetting +x for \e[35m$SETUP_BASEDIR\e[36m.\e[m";
				if chmod +x "$SETUP_BASEDIR" > /dev/null 2>&1;
				then
					echo -e "\e[36m------> \e[1mSuccessfully made \e[35m$SETUP_BASEDIR\e[36m executable.\e[m";
				else
					echo -e "\e[41m----EE> \e[1mCould not chmod +x on \e[33m$SETUP_BASEDIR\e[m\e[1;41m.\e[m" >&2;
					echo -e "\e[1;31mPlease correct the errors above and run again.\e[m" >&2;
					exit 1;
				fi;
			#
		fi;
	#
	# is at least one file in setup.d executable?
	#
		FILE_NUM=0;
		for BASEDIR_FILE in ${SETUP_BASEDIR}/*;
		do
			if [ -e "$BASEDIR_FILE" ];
			then
				if [ -d "$BASEDIR_FILE" ];
				then
					echo -e "\e[31m--WW> \e[1;35m$BASEDIR_FILE\e[31m is a directory.\e[m" >&2;
				elif [ -x "$BASEDIR_FILE" ];
				then
					echo -e "\e[36m----> \e[1;35m$BASEDIR_FILE\e[36m exists and is executable.\e[m";
					FILE_NUM=$(expr $FILE_NUM + 1);
				else
					echo -e "\e[31m--WW> \e[1;35m$BASEDIR_FILE\e[31m exists, but is not executable.\e[m" >&2;
					#
					# chmod +x if it's possible.
					#
						echo -e "\e[36m------> \e[1mSetting +x for \e[35m$BASEDIR_FILE\e[36m.\e[m";
						if chmod +x "$BASEDIR_FILE" > /dev/null 2>&1;
						then
							echo -e "\e[36m------> \e[1mSuccessfully made \e[35m$BASEDIR_FILE\e[36m executable.\e[m";
							FILE_NUM=$(expr $FILE_NUM + 1);
						else
							echo -e "\e[31m----WW> \e[1mCould not chmod +x on \e[35m$BASEDIR_FILE\e[31m.\e[m" >&2;
						fi;
					#
				fi;
			fi;
		done;
		#
		# Check the number of existing files
		#
		if [ $FILE_NUM -ge 1 ];
		then
			echo -e "\e[36m----> \e[1;35m$FILE_NUM\e[36m files available in \e[35m$SETUP_BASEDIR\e[36m.\e[m";
		else
			echo -e "\e[41m--EE> \e[1m\e[33m$SETUP_BASEDIR\e[m\e[1;41m is not usable.\e[m" >&2;
			echo -e "\e[1;31mPlease correct the errors above and run again.\e[m" >&2;
			exit 1;
		fi;
	#
# }}}

# Check configuration file {{{
	echo -e "\e[33m--> \e[1mIs configuration okay?\e[m";
	#
	# Does it exist?
	#
		if ! echo "$CONFIG_FILE"|grep '^/' > /dev/null 2>&1;
		then
			CONFIG_FILE="$PWD/$CONFIG_FILE";
		fi
		CONFIG_FILE="$(echo -n "$CONFIG_FILE"|sed -e 's|/\.*/|/|g')";
		if [ -e "$CONFIG_FILE" ];
		then
			echo -e "\e[36m----> \e[1mFile is \e[35m$CONFIG_FILE\e[36m and exists.\e[m";
		else
			echo -e "\e[41m--EE> \e[1mWhere is \e[33m$CONFIG_FILE\e[m\e[1;41m?\e[m" >&2;
			echo -e "\e[1;31mPlease correct the errors above and run again.\e[m" >&2;
			exit 1;
		fi;
	#
	# Regular file or symbolic link?
	#		
		if [ -h "$CONFIG_FILE" ];
		then
			echo -en "\e[36m----> \e[1mIt is a symbolic link...\e[m ";
		elif [ -f "$CONFIG_FILE" ]
		then
			echo -en "\e[36m----> \e[1mIt is a regular file...\e[m ";
		else
			echo -e "\e[41m--EE> \e[1mIt's not a symbolic link nor a regular file!\e[m" >&2;
			echo -e "\e[1;31mPlease correct the errors above and run again.\e[m" >&2;
			exit 1;
		fi;
	#
	# Readable?
	#
		if [ -r "$CONFIG_FILE" ];
		then
			echo -e "\e[1;36mand readable.\e[m";
		else
			echo -e "\e[41;1mand is NOT READABLE.\e[m" >&2;
			echo -e "\e[1;31mPlease correct the errors above and run again.\e[m" >&2;
			exit 1;
		fi
	#
# }}}

echo -e "\e[1;32mOkay, starting setup scripts:\e[m";

export CONFIG_FILE;

run-parts "$SETUP_BASEDIR";
