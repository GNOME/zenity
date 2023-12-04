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
	say join(', ', keys %tests);
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

	say "$cmd";
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

sub test_cmd_for_stdout
{
	my ($cmd, $expected_stdout) = @_;
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

create_test ("test_entry",
	"Press enter or OK",
	sub {
		my $expected_stdout = 'foo bar baz';
		my $cmd = "$ZENITY --entry --entry-text='$expected_stdout'";

		test_cmd_for_stdout ($cmd, $expected_stdout);
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
	foreach my $key (keys %tests)
	{
		$tests{$key}();
	}
}

test_summary();
