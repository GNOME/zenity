#!/usr/bin/env perl

# vim: ts=4 sw=4 linebreak breakindent breakindentopt=shift\:4

use strict;
use warnings;
use v5.12;

my $ZENITY = $ENV{ZENITY} || 'zenity';

my %tests;
my $n_tests_passed = 0;
my $n_tests_failed = 0;

my @all_dialogs = (qw[
	--calendar
	--entry
	--error
	--info
	--file-selection
	--list
	--notification
	--progress
	--question
	--warning
	--scale
	--text-info
	--color-selection
	--password
	--forms
	]);

my @all_dialogs_except_notification = @all_dialogs;
{
	for (my $index = 0; $all_dialogs_except_notification[$index]; ++$index)
	{
		if ($all_dialogs_except_notification[$index] eq '--notification') {
			splice @all_dialogs_except_notification, $index, 1;
			last;
		}
	}
}

sub get_dialogs_with_plain_vanilla_options
{
	my $arrRef = shift;
	my @retval;

	foreach my $d (@$arrRef)
	{
		# special case: list & notification - can't be run without arguments.
		if ($d eq '--list') {
			$d .= ' --column=foo foo';
		} elsif ($d eq '--notification') {
			$d .= ' --text=test';
		# special case: progress - can't click ok until percentage full
		} elsif  ($d eq '--progress') {
			$d .= ' --percentage=100';
		}

		push @retval, $d;
	}

	return @retval;
}

sub usage
{
	say "Usage: '$0 [test_name]' or '$0' with no arguments to run all tests. '$0 --help' to list tests.";
	exit 1;
}

sub print_tests_and_exit
{
	say "Available tests:";
	say join(', ', sort keys %tests);
	exit 1;
}

sub test_passed
{
	my $test_name = shift;

	my $str = "\tPASSED";
	if (defined $test_name) {
		$str = "$test_name: $str";
	}

	say $str;
	++$n_tests_passed;
}

sub test_failed
{
	my $test_name = shift;

	my $str = "\tFAILED";
	if (defined $test_name) {
		$str = "$test_name: $str";
	}

	say $str;
	++$n_tests_failed;
}

sub test_summary
{
	say '***** TEST SUMMARY *****';
	say "Tests passed: $n_tests_passed";
	say "Tests failed: $n_tests_failed";

	if ($n_tests_failed > 0) {
		exit 1;
	} else {
		exit 0;
	}
}

sub get_exit_status
{
	my $raw = $?;
	return $raw ? $raw >> 8 : 0;
}

sub create_test
{
	my ($test_basename, $test_desc, $test_subRef) = @_;
	my $subRef = undef;

	$subRef = sub {
		say "***** RUNNING TEST: $test_basename *****";
		say "\t$test_desc";
		&$test_subRef ();
		say "***** TEST FINISHED *****";
		say '';
	};

	$tests{$test_basename} = $subRef;
}

sub test_cmd_for_exit_status
{
	my ($cmd, $target_exit_status) = @_;
	my $output;
	my $exit_status;

	say "Running command: $cmd";
	$output = `$cmd >/dev/null 2>&1`;
	$exit_status = get_exit_status ();
	say "\ttarget exit status: $target_exit_status";
	say "\tgot exit status: $exit_status";

	if ($exit_status == $target_exit_status) {
		test_passed ();
	} else {
		test_failed ();
	}
}

sub sort_multi_stdout_list {
	my ($input_string, $separator) = @_;

	if (length($separator) != 1) {
		warn "Invalid separator: $separator (must be one character)\n";
		return;
	}

	my @matches = split /\Q$separator\E/, $input_string;
	my @sorted_matches = sort @matches;
	my $output_string = join $separator, @sorted_matches;

	return $output_string;
}

sub test_cmd_for_stdout
{
	my ($cmd, $expected_stdout, $multi_separator) = @_;

	say "Running command: $cmd";
	my $stdout = `$cmd`;

	chomp $stdout;

	if ($multi_separator) {
		$stdout = sort_multi_stdout_list ($stdout, $multi_separator);
		$expected_stdout = sort_multi_stdout_list ($expected_stdout, $multi_separator);
	}

	say "\t\tExpected stdout: $expected_stdout";
	say "\t\tGot stdout: $stdout";

	if ($stdout eq $expected_stdout) {
		test_passed ();
	} else {
		test_failed ();
	}
}

sub do_twice_in_parallel
{
	my $cmd = shift;

	print "Performing twice in parallel: $cmd\t\t";

	my %retval;
	my $pid = fork;

	die "$0: Could not fork" unless defined $pid;

	# child
	if ($pid == 0) {
		my $dummy_stdout = `$cmd`;
		my $exit_status;

		$exit_status = get_exit_status ();

		exit $exit_status;
	}
	# parent
	else {
		my $dummy_stdout = `$cmd`;

		$retval{exit_status1} = get_exit_status ();

		waitpid $pid, 0;
		$retval{exit_status2} = get_exit_status ();
	}

	print "[DONE]\n";

	return %retval;
}

create_test ("timeout",
	"Please allow these dialogs to close themselves while the test runs.",
	sub {
		my $timeout_exit_status = 5;
		my @dialogs = get_dialogs_with_plain_vanilla_options (\@all_dialogs);

		foreach my $d (@dialogs)
		{
			my $test = "$ZENITY $d --timeout=1";

			test_cmd_for_exit_status ($test, $timeout_exit_status);
		}
	}
);

create_test ("checklist_search",
	"Type 'fruit', select all checkboxes that appear, and click OK",
	sub {
		my $cmd = "$ZENITY --width=640 --height=480 --list --checklist --column=Check --column=Produce FALSE 'Apple fruit' FALSE 'Orange fruit' FALSE 'Carrot veg' FALSE 'Celery veg'";
		my $expected_stdout = 'Apple fruit|Orange fruit';

		test_cmd_for_stdout ($cmd, $expected_stdout);
	}
);

create_test ("radiolist_search",
	"Type 'fruit', select the first (and only) radio button that appears, and click OK",
	sub {
		my $cmd = "$ZENITY --width=640 --height=480 --list --radiolist --column=Radio --column=Produce FALSE 'Onion veg' FALSE 'Apple fruit' FALSE 'Carrot veg' FALSE 'Celery veg'";
		my $expected_stdout = 'Apple fruit';

		test_cmd_for_stdout ($cmd, $expected_stdout);
	}
);

create_test ("list_search_multi",
	"Type 'fruit', select all list items that appear, and click OK",
	sub {
		my $cmd = "$ZENITY --width=640 --height=480 --list --multiple --print-column=2 --column=Junk --column=Produce Candy 'Apple fruit' Chocolate 'Orange fruit' Chips 'Carrot veg' Fries 'Celery veg' Soda 'Grapefruit juice'";
		my $expected_stdout = 'Apple fruit|Orange fruit|Grapefruit juice';
		my $stdout = `$cmd`;

		chomp $stdout;

		say "\t\tExpected stdout: $expected_stdout";
		say "\t\tGot stdout: $stdout";

		if ($stdout eq $expected_stdout) {
			test_passed ();
		} else {
			test_failed ();
		}
	}
);

create_test ("ok_button",
	"Click the affirmative (OK, Yes, etc) on all dialogs that appear.",
	sub {
		foreach my $d (get_dialogs_with_plain_vanilla_options (\@all_dialogs_except_notification))
		{
			my $test = "$ZENITY $d";
			my $ok_exit_status = 0;
			my $custom_exit_status = 111;

			test_cmd_for_exit_status ($test, $ok_exit_status);

			$test = "ZENITY_OK=$custom_exit_status $test";
			test_cmd_for_exit_status ($test, $custom_exit_status);
		}
	}
);

create_test ("custom_title",
	"Check the dialogs to ensure there is a custom title visible called 'FOOBAR'.\nIf so, click the affirmative (OK, Yes, etc) on all dialogs that appear.",
	sub {
		foreach my $d (get_dialogs_with_plain_vanilla_options (\@all_dialogs_except_notification))
		{
			my $test = "$ZENITY $d --title='FOOBAR'";
			my $ok_exit_status = 0;

			test_cmd_for_exit_status ($test, $ok_exit_status);
		}
	}
);

create_test ("esc_or_close",
	"Close the dialogs (or hit the ESC key) on all dialogs that appear",
	sub {
		foreach my $d (get_dialogs_with_plain_vanilla_options (\@all_dialogs_except_notification))
		{
			my $test = "$ZENITY $d";
			my $esc_exit_status = 1;
			my $custom_exit_status = 222;

			test_cmd_for_exit_status ($test, $esc_exit_status);

			$test = "ZENITY_ESC=$custom_exit_status $test";
			test_cmd_for_exit_status ($test, $custom_exit_status);
		}
	}
);

create_test ("cancel_button",
	"Click the negative (Cancel, No, etc.) on all dialogs that appear",
	sub {
		# easier to just specify these dialogs which have cancel buttons manually :-(
		my @dialogs = qw[--calendar --entry --file-selection --list --progress --scale --color-selection --password --forms];
		foreach my $d (get_dialogs_with_plain_vanilla_options (\@dialogs))
		{
			my $test = "$ZENITY $d";
			my $cancel_exit_status = 1;
			my $custom_exit_status = 222;

			test_cmd_for_exit_status ($test, $cancel_exit_status);

			$test = "ZENITY_CANCEL=$custom_exit_status $test";
			test_cmd_for_exit_status ($test, $custom_exit_status);
		}
	}
);

create_test ("custom_ok_cancel_buttons",
	"Ensure that both the OK and Cancel buttons contain a frowny- or smiley-faced cat emoji respetively, and click the smiley face. If either okay or cancel do not contain the kitty emoji, click cancel or close the window.",
	sub {
		# mostly the same as the Cancel dialogs - but custom ok/cancel labels is NOT supported for --file-selection
		my @dialogs = qw[--calendar --entry --list --progress --scale --color-selection --password --forms];
		foreach my $d (get_dialogs_with_plain_vanilla_options (\@dialogs))
		{
			my $test = qq[$ZENITY $d --ok-label="😺" --cancel-label="😾"];
			my $expected_exit_status = 0;

			test_cmd_for_exit_status ($test, $expected_exit_status);
		}
	}
);

create_test ("test_entry",
	"Follow the instructions on the dialogs",
	sub {
		my $expected_stdout = 'foo bar baz';
		my $cmd1 = "$ZENITY --entry --text='Click OK' --entry-text='$expected_stdout'";

		test_cmd_for_stdout ($cmd1, $expected_stdout);

		my $cmd2 = "$ZENITY --entry --hide-text --text='Click OK if you see dots; click Cancel if you see plaintext' --entry-text='$expected_stdout'";

		test_cmd_for_stdout ($cmd2, $expected_stdout);
	}
);

create_test ("test_text_info",
	"Type 'foobar' in the textview and click OK",
	sub {
		my $expected_stdout = 'foobar';
		my $cmd = "$ZENITY --text-info --editable";

		test_cmd_for_stdout ($cmd, $expected_stdout);
	}
);

create_test ("test_text_info_font",
	"Type 'foobar' in the textview.\n\tClick OK the text is monospace. Otherwise, click Cancel or close window.",
	sub {
		my $expected_stdout = 'foobar';
		my $cmd = "$ZENITY --text-info --editable --font=monospace";

		test_cmd_for_stdout ($cmd, $expected_stdout);
	}
);

create_test ("multiple_zenity_instances",
	"Wait for both dialogs to terminate",
	sub {
		my $expected_exit_status = 5;
		my $cmd = "$ZENITY --info --timeout=3";
		my %retval = do_twice_in_parallel ($cmd);

		say "\t\tExpected exit status: $expected_exit_status";
		say "\t\tGot exit status #1: $retval{exit_status1}";
		say "\t\tGot exit status #2: $retval{exit_status2}";

		if ($expected_exit_status == $retval{exit_status1} == $retval{exit_status2})
		{
			test_passed ();
		}
		else
		{
			test_failed ();
		}
	}
);

create_test ("issue_72_list_infloop",
	"Double-click the item in the list within 20 seconds (this test only applicable to GTK 4.12+)",
	sub {
		my $expected_exit_status = 0;
		my $cmd = <<"EOF";

# Running the following script:
$ZENITY --list --column=foo foo &
pid=\$!

sleep 20

if ps -p \$pid >/dev/null; then
  kill -TERM \$pid
  exit 1
else
  exit 0
fi
EOF
		test_cmd_for_exit_status ($cmd, 0);
	}
);

# --list has a LOT of options. Let's make sure they actually work.
create_test ("list_o_mania",
	"Change the 3 items in this list to foo, bar and baz respectively, and check all the boxes.",
	sub {
		my $cmd = qq[$ZENITY --list --width=640 --height=480 --checklist --editable --column=Check --column=Crap --column=Item --print-column=2,3 --hide-header --hide-column=2 --text="Follow the instructions on the terminal.\nThis dialog should have checkboxes but should NOT have headers.\nIt should contain only the checkbox and the words Alpha, Beta and Gamma.\nIf any of this isn't the case, click Cancel or close the window." --separator=',' FALSE Bleh Alpha FALSE Meh Beta FALSE Blah Gamma];
		test_cmd_for_stdout ($cmd, 'Bleh,foo,Meh,bar,Blah,baz');
	}
);

# ... same with --forms
create_test ("forms_options",
	"For 'My entry', enter 'foo'; for 'My password' enter 'bar'; for 'My text info' enter 'baz<return>boo'\n\tKeep today's date selected for the calendar; select the first item in the list;\n\tselect the second item in both dropdowns.",
	sub {
		my $cmd = qq[$ZENITY --forms --add-entry='My entry' --add-password='My password' --add-calendar='My calendar' --add-list='My list' --list-values='foo bar|baz boo|bleh blah|boo urns' --column-values='foo|bar' --add-combo='My combo box' --combo-values='combo1|combo2|combo3' --add-combo='My other combo box' --combo-values='combo4|combo5|combo6' --show-header --text="My Form" --separator='^' --forms-date-format='%F' --add-multiline-entry='My text info'];
		my $iso_date = `date --iso-8601`;
		chomp $iso_date;
		my $expected_stdout = "foo^bar^$iso_date^foo bar,baz boo,^combo2^combo5^baz\nboo";

		test_cmd_for_stdout ($cmd, $expected_stdout);
	}
);

create_test ("test_progress",
	"Allow these progress dialogs to close themselves unless the dialog says otherwise",
	sub {
		my $ok_exit_status = 0;
		my $cancel_exit_status = 1;
		my $error_exit_status = 255;
		my $pulsate_opt_cmd = "sleep 3 | $ZENITY --progress --pulsate --auto-close";

		test_cmd_for_exit_status ($pulsate_opt_cmd, $ok_exit_status);

		# --auto-close and --percentage=100 not supported.
		my $percentage_100_cmd = "$ZENITY --progress --auto-close --percentage=100";

		test_cmd_for_exit_status ($percentage_100_cmd, $error_exit_status);

		my $cancel_progress_cmd = "ping 127.0.0.1 | $ZENITY --progress --text='Click Cancel please' --pulsate";

		test_cmd_for_exit_status ($cancel_progress_cmd, $cancel_exit_status);

		my $auto_close_cmd = <<"EOF";
(
sleep 5;
echo "10" ; sleep 2
echo "# Blah1" ; sleep 2
echo "20" ; sleep 2
echo "# Blah2" ; sleep 2
echo "50" ; sleep 2
echo "If you see this line, click Cancel" ; sleep 4
echo "75" ; sleep 2
echo "# Blah3" ; sleep 2
echo "100" ; sleep 2
) |
$ZENITY --progress \\
  --auto-close \\
  --percentage=0 \\
  --text='Let this dialog auto-close.\\n\\nTime remaining should be shown in a few seconds after the progress bar starts.\\n\\nIf time remaining is not shown by \`Blah3\`, click Cancel.' \\
  --time-remaining
EOF

		test_cmd_for_exit_status ($auto_close_cmd, $ok_exit_status);

		my $pulsate_from_stdin_cmd = <<"EOF";
(
echo pulsate: true
sleep 5
echo pulsate: false
echo 99
sleep 1
echo 100
) |
$ZENITY --progress --auto-close --text='Progress should pulsate for about 5 seconds.\\n\\nIf not, click Cancel.'
EOF
		test_cmd_for_exit_status ($pulsate_from_stdin_cmd, $ok_exit_status);
	}
);

create_test ("test_icon",
	"Click OK (or Yes/affirmative answer) if you see a star icon on these dialogs",
	sub {
		my $expected_exit_status = 0;
		foreach my $opt ('--error', '--info', '--question', '--warning')
		{
			my $cmd = "$ZENITY $opt --icon=starred";
			test_cmd_for_exit_status ($cmd, $expected_exit_status);
		}
	}
);

create_test ("test_ellipsize",
	"If you see a '...' at the end of the dialog text, click OK (or Yes/affirmative answer).\n\n\tOtherwise, click Cancel or close the dialog.",
	sub {
		my $expected_exit_status = 0;
		foreach my $opt ('--error', '--info', '--question', '--warning')
		{
			my $cmd = "$ZENITY $opt --text='asfdsafdsfadsfdsafasdfadsfdsafadsfdsafadsfsadfasdfasdfsadfadsfadsfadsfadsfadsfadsfadsfadsfadsf' --ellipsize";
			test_cmd_for_exit_status ($cmd, $expected_exit_status);
		}
	}
);

create_test ("test_no_markup",
	"If you see a visible '\\n', click OK (or affirmative answer).\n\n\tIf you see a linebreak, click Cancel/negative answer/close the dialog.",
	sub {
		my $expected_exit_status = 0;
		foreach my $opt ('--error', '--info', '--question', '--warning')
		{
			my $cmd = "$ZENITY $opt --no-markup --text='foo\\nbar'";
			test_cmd_for_exit_status ($cmd, $expected_exit_status);
		}
	}
);

create_test ("issue_75_ensure_num_entries",
	"3 entry dialogs will appear. For #1, just click OK.\n\tFor #2, select the 2nd item and click OK.\n\tFor #3, select the 3rd item and click OK.\n\tIf there is no 2nd (or 3rd) item for #2 or #3, click Cancel.",
	sub {
		my $expected_stdout = 'foo bar';
		my $cmd1 = "$ZENITY --entry --entry-text 'foo bar'";
		my $cmd2 = "$ZENITY --entry --entry-text 'baz boo' 'foo bar'";
		my $cmd3 = "$ZENITY --entry --entry-text 'bleh bah' 'baz boo' 'foo bar'";

		foreach ($cmd1, $cmd2, $cmd3) {
			test_cmd_for_stdout ($_, $expected_stdout);
		}
	}
);

create_test ("test_calendar",
	"Some calendars will appear. Follow the instructions on each dialog.",
	sub {
		my $iso_date = `date --iso-8601`;
		chomp $iso_date;
		my $base_cmd = "$ZENITY --calendar --date-format='%F'";

		test_cmd_for_stdout (qq[$base_cmd --text="Ensure today's date is selected and click OK. If the date is wrong, click Cancel."], $iso_date);
		test_cmd_for_stdout (qq[$base_cmd --text="Select November 11, 1985 and click OK."], '1985-11-11');
		test_cmd_for_stdout (qq[$base_cmd --text="Click OK." --day=29 --month=2 --year=2024], '2024-02-29');
	}
);

create_test ("file_selection",
	"Some file selection dialogs will appear.\n\tFollow the instructions that appear in the console for each dialog.\n\tYou will need write permission to /tmp for this test to work.",
	sub {
		my $tmpfile;
		my $expected_stdout;
		my $base_cmd = "$ZENITY --file-selection";

		chomp($tmpfile = `mktemp /tmp/zenity_test.XXXXXXXXXX`);
		say "\nClick OK. If OK is greyed out, click Cancel.";
		test_cmd_for_stdout (qq[$base_cmd --filename="$tmpfile"], "$tmpfile");
		unlink $tmpfile;

		chomp($tmpfile = `mktemp /tmp/zenity_test.XXXXXXXXXX`);
		unlink $tmpfile;
		say "\nClick OK. If OK is greyed out, click Cancel.";
		test_cmd_for_stdout (qq[$base_cmd --save --filename="$tmpfile"], "$tmpfile");

		chomp($tmpfile = `mktemp /tmp/zenity_test.XXXXXXXXXX`);
		unlink $tmpfile;
		say "\nClick OK. If OK is greyed out, click Cancel.";
		test_cmd_for_stdout (qq[$base_cmd --save --filename="file://$tmpfile"], "$tmpfile");

		chomp($tmpfile = `mktemp -d /tmp/zenity_test.XXXXXXXXXX`);
		say "\nEnter abcd then Click OK. If OK is greyed out, click Cancel.";
		test_cmd_for_stdout (qq[$base_cmd --save --filename="$tmpfile/"], "${tmpfile}/abcd");
		rmdir $tmpfile;

		chomp($tmpfile = `mktemp -d /tmp/zenity_test.XXXXXXXXXX`);
		say "\nEnter abcd then Click OK. If OK is greyed out, click Cancel.";
		test_cmd_for_stdout (qq[$base_cmd --save --filename="file://$tmpfile/"], "${tmpfile}/abcd");
		rmdir $tmpfile;

		my $tmpfile2;

		chomp($tmpfile = `mktemp /tmp/zenity_test.XXXXXXXXXX`);
		chomp($tmpfile2 = `mktemp /tmp/zenity_test.XXXXXXXXXX`);
		say "\nSelect BOTH temporary files (one has been pre-selected) and click OK.";
		test_cmd_for_stdout (qq[$base_cmd --multiple --file-filter="zenity_test.*" --filename="$tmpfile"], "${tmpfile}|${tmpfile2}", '|');
		unlink $tmpfile;
		unlink $tmpfile2;

		chomp($tmpfile = `mktemp /tmp/zenity_test.XXXXXXXXXX`);
		chomp($tmpfile2 = `mktemp /tmp/zenity_test.XXXXXXXXXX`);
		say "\nSelect BOTH temporary files (one has been pre-selected) and click OK.";
		test_cmd_for_stdout (qq[$base_cmd --multiple --file-filter="zenity_test.*" --separator="~" --filename="$tmpfile"], "${tmpfile}~${tmpfile2}", '~');
		unlink $tmpfile;
		unlink $tmpfile2;
	}
);

# MAIN

if ($ARGV[0])
{
	if ($ARGV[0] eq '--help') {
		print_tests_and_exit ();
	}

	foreach my $arg (@ARGV)
	{
		if ($tests{$arg}) {
			$tests{$arg}();
		} else {
			usage ();
		}
	}
}
else
{
	foreach my $key (sort keys %tests)
	{
		$tests{$key}();
	}
}

test_summary();
