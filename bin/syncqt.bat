@rem = '--*-PERL-*--';
@rem = '
@echo off
rem setlocal
set ARGS=
:loop
if .%1==. goto endloop
set ARGS=%ARGS% %1
shift
goto loop
:endloop
rem ***** This assumes PERL is in the PATH *****
perl.exe -S syncqt.bat %ARGS%
goto endofperl
@rem ';
#!/usr/bin/perl -w
############################################################################
# $Id: $
#
# Synchronizes Qt header files - internal Trolltech tool.
#
# Copyright (C) 1997-%THISYEAR% by Trolltech AS.  All rights reserved.
#
############################################################################

use File::Basename;
use Config;
use strict;

#required
die "syncqt: QTDIR not defined" if ! $ENV{"QTDIR"};

#global
my $basedir = $ENV{"QTDIR"};
my $includedir = $basedir . "/include";

#options
my $showonly = 0;
my $force_win=0;
my $force_relative=0;
$force_relative=1 if ( -d "/System/Library/Frameworks" );

while($#ARGV >= 0) {
    if($ARGV[0] eq "-show") {
	$showonly++;
    } elsif($ARGV[0] eq "-outdir" || $ARGV[0] eq "-inc") {
	shift;
	my $outdir = $ARGV[0];
	if($outdir !~ /^\//) {
	    $includedir = `pwd`;
	    chomp $includedir;
	    $includedir .= "/" . $outdir;
	} else {
	    $includedir = $outdir;
	}
    } elsif($ARGV[0] eq "-windows") {
	$force_win=1;
    }
    shift;
}

$basedir =~ s=\\=/=g;
$includedir =~ s=\\=/=g;

mkdir $includedir, 0777;

my @ignore_headers = ();
my @ignore_master_headers = ( "qt.h", "qpaintdevicedefs.h" );

my %dirs = (
	 "src/gui" => "QtGUI",
	 "src/opengl" => "QtOpenGL",
	 "src/core" => "QtCore",
	 "src/xml" => "QtXML",
	 "src/sql" => "QtSQL",
	 "src/network" => "QtNetwork",
	 "src/canvas" => "QtCanvas",
	 "src/compat" => "Qt3Compat",
);

$dirs{"mkspecs" . $ENV{"MKSPEC"}} = "QCore" if defined $ENV{"MKSPEC"}; 

my $dir;
foreach $dir (keys %dirs) {
    my $lib = $dirs{$dir};

    #calc subdirs
    my @subdirs = ($dir);
    foreach (@subdirs) {
	my $subdir = "$_";
	opendir DIR, "$subdir";
	while(my $t = readdir(DIR)) {
	    push @subdirs, "$subdir/$t" if(-d "$subdir/$t" && !($t eq ".") && !($t eq "..") && !($t eq "arch"));
	}
	closedir DIR;
    }

    #calc files and "copy" them
    my $master_header_out = "";
    foreach (@subdirs) {
	my $subdir = "$_";
	my @headers = find_files("$subdir", "^[-a-z0-9_]*\\.h\$" , 0);
	foreach (@headers) { 
	    my $header = "$_"; 
	    foreach (@ignore_headers) { 
		$header = 0 if("$header" eq "$_");
	    }
	    if("$header") {
		my $iheader = $basedir . "/" . $subdir . "/" . $header;
		if($showonly) {
		    print "$header [$lib]\n";
		} else {
		    #figure out if this belongs in the master file
		    unless($header =~ /_/) {
			my $public_header = $header;
			foreach (@ignore_master_headers) { 
			    $public_header = 0 if("$header" eq "$_");
			}
			$master_header_out .= "#include \"$public_header\"\n" if($public_header);
		    }
		    #now sync the header file (ie symlink/include it)
		    sync_header($header, $iheader, $lib);
		}
		sync_classnames($iheader, $lib) unless($header =~ /_/);
	    }
	}
    }

    unless($showonly) {
	#finally generate the "master" include file
	my $master_include = "$includedir/$lib/$lib";
	if(-e "$master_include") {
	    open MASTERINCLUDE, "<$master_include";
	    local $/;
	    binmode MASTERINCLUDE;
	    my $oldmaster = <MASTERINCLUDE>;
	    close MASTERINCLUDE;
	    $master_include = 0 if($oldmaster eq $master_header_out);
	}
	if($master_include) {
	    print "header (master) created for $lib\n";
	    open MASTERINCLUDE, ">$master_include";
	    print MASTERINCLUDE "$master_header_out";
	    close MASTERINCLUDE;
	}
    }
}


if(!check_unix()) {
   symlink_file("$basedir/dist/win/Makefile", "$basedir/Makefile");
   symlink_file("$basedir/dist/win/Makefile.win32-g++", "$basedir/Makefile.win32-g++");
}

exit 0;

# sync_classnames(iheader, library)
#
#
# all classnames found in iheader are synced into library's include structure
#
sub sync_classnames {
    my $ret = 0;
    my ($iheader, $library) = @_;

    my $parsable = "";
    if(open(F, "<$iheader")) {
	while(<F>) {
	    my $line = $_;
	    chomp $line;
	    if($line =~ /^\#/) {
		if($line =~ /\\$/) {
		    while($line = <F>) {
			chomp $line;
			last unless($line =~ /\\$/);
		    }
		} else {
		    $line = 0;
		}
	    }
	    $line =~ s,//.*$,,; #remove c++ comments
	    $parsable .= $line if($line);
	}
	close(F);
    }

    my $last_definition = 0;
    for(my $i = 0; $i < length($parsable); $i++) {
	my $definition = 0;
	my $character = substr($parsable, $i, 1);
	if($character eq "/" && substr($parsable, $i+1, 1) eq "*") { #I parse like this for greedy reasons
	    for($i+=2; $i < length($parsable); $i++) {
		my $end = substr($parsable, $i, 2);
		if($end eq "*/") {
		    $last_definition = $i+2;
		    $i++;
		    last;
		}
	    }
	} elsif($character eq "{") {
	    my $brace_depth = 1;
	    my $block_start = $i + 1;
	  BLOCK: for($i+=1; $i < length($parsable); $i++) {
	      my $ignore = substr($parsable, $i, 1);
	      if($ignore eq "{") {
		  $brace_depth++;
	      } elsif($ignore eq "}") {
		  $brace_depth--;
		  unless($brace_depth) {
		      for(my $i2 = $i+1; $i2 < length($parsable); $i2++) {
			  my $end = substr($parsable, $i2, 1);
			  if($end eq ";" || $end ne " ") {
			      $definition = substr($parsable, $last_definition, $block_start - $last_definition) . "}";
			      $i = $i2 if($end eq ";");
			      $last_definition = $i + 1;
			      last BLOCK;
			  }
		      }
		  }
	      }
	  }
	} elsif($character eq ";") {
	    $definition = substr($parsable, $last_definition, $i - $last_definition + 1);
	    $last_definition = $i + 1;
	}
	if($definition) {
	    my $symbol = 0;
	    if($definition =~ m/^typedef *.*\(\*([^\)]*)\)\(.*\);$/) {
		$symbol = $1;
	    } elsif($definition =~ m/^typedef +(.*) +([^ ]*);$/) {
		$symbol = $2;
	    } elsif($definition =~ m/^(template<.*> *)?(class|struct) +([^ ]* +)?([^ ]+) ?((,|:) *(public|private) *.*)? *\{\}$/) {
		$symbol = $4;
	    }
	    if($symbol && $symbol =~ /^Q/) {
		$ret++;
		if($showonly) {
		    print "SYMBOL: $symbol\n";
		} else {
#		    print "$iheader: $symbol\n";
		}
	    }
	}
    }
    print "SYMBOLS: Found $ret\n" if($showonly);
    return $ret;
}

#
# sync_header(header, iheader, library)
#
# header is synconized to iheader
#
sub sync_header {
    my ($header, $iheader, $library) = @_;
    $iheader =~ s=\\=/=g;
    $header =~ s=\\=/=g;
    if(-e "$iheader") {
	my $iheader_no_basedir = $iheader;
	$iheader_no_basedir =~ s,^$basedir/?,,;

	my $headers_created = 0;
	my @headers_out;
	if (($header =~ /_p.h$/)) {
	    @headers_out = ( "$includedir/Qt/private/$header", "$includedir/$library/private/$header" );
	} else {
	    @headers_out = ( "$includedir/Qt/$header", "$includedir/$library/$header" );
	}
	unlink "$includedir/$header" if(-e "$includedir/$header"); #remove old symlink from 3.x
	foreach(@headers_out) { 
	    my $header_out = $_;
	    if(!-e "$header_out") {
		$headers_created++;

		my $header_out_dir = dirname($header_out);
		mkdir $header_out_dir, 0777;

		#write it
		my $iheader_out = fix_paths($iheader, $header);
		open HEADER, ">$header_out" || die "Could not open $header_out for writing!\n";
		print HEADER "#include \"$iheader_out\"\n";
		close HEADER;
	    }
	}
	print "header created for $iheader_no_basedir\n" if($headers_created > 0);
    }
}

#
# fix_paths(file1, file2)
#
# 
# file1 is made relative (if possible) of file2 and returned
sub fix_paths {
    my ($file1, $file2) = @_;
    $file1 =~ s=\\=/=g;
    $file2 =~ s=\\=/=g;

    my $ret = $file1;
    my $file1_dir = dirname($file1);
    my $file2_dir = dirname($file2);

    #guts
    my $match_dir = 0;
    for(my $i = 1; $i < length($file1_dir); $i++) {
	my $tmp = substr($file1_dir, 0, $i);
	last unless($file2_dir =~ /^$tmp/);
	$match_dir = $tmp;
    }
    if($match_dir) {
	my $after = substr($file2_dir, length($match_dir));
	my $count = ($after =~ tr,/,,);
	my $dots = "";
	for(my $i = 0; $i <= $count; $i++) {
	    $dots .= "../";
	}
	$ret =~ s,^$match_dir,$dots,;
    }
    return $ret;
}

#
# symlink_file(file,ifile)
#
# file is symlinked to ifile (or copied if filesystem doesn't support symlink)
#
sub symlink_file
{
    my ($file,$ifile, $fast,$copy,$knowdiff,$filecontents,$ifilecontents) = @_;

    if (check_unix()) {
	print "symlink created for $file ";
	if ( $force_relative && ($ifile =~ /^$basedir/)) {
	    my $t = `pwd`; 
	    my $c = -1; 
	    my $p = "../";
	    $t =~ s-^$basedir/--;
	    $p .= "../" while( ($c = index( $t, "/", $c + 1)) != -1 );
	    $file =~ s-^$basedir/-$p-;
	    print " ($file)\n";
	}
	print "\n";
	symlink($file, $ifile);
	return;
    } else {
	# Bi-directional synchronization
	open( I, "< " . $file ) || die "Could not open $file for reading";
	local $/;
	binmode I;
	$filecontents = <I>;
	close I;
	if ( open(I, "< " . $ifile) ) {
	    local $/;
	    binmode I;
	    $ifilecontents = <I>;
	    close I;
	    $copy = (stat($ifile))[9] <=> (stat($file))[9];
	    $knowdiff = 0,
	} else {
	    $copy = -1;
	    $knowdiff = 1;
	}
    }

    if ( $knowdiff || ($filecontents ne $ifilecontents) ) {
	if ( $copy > 0 ) {
	    open(O, "> " . $file) || die "Could not open $file for writing";
	    local $/;
	    binmode O;
	    print O $ifilecontents;
	    close O;
	    print "$file written\n";
	} elsif ( $copy < 0 ) {
	    open(O, "> " . $ifile) || die "Could not open $ifile for writing";
	    local $/;
	    binmode O;
	    print O $filecontents;
	    close O;
	    print "$ifile written\n";
	}
    }
}


#
# Finds files.
#
# Examples:
#   find_files("/usr","\.cpp$",1)   - finds .cpp files in /usr and below
#   find_files("/tmp","^#",0)	    - finds #* files in /tmp
#
sub find_files {
    my($dir,$match,$descend) = @_;
    my($file,$p,@files);
    local(*D);
    $dir =~ s=\\=/=g;
    ($dir eq "") && ($dir = ".");
    if ( opendir(D,$dir) ) {
	if ( $dir eq "." ) {
	    $dir = "";
	} else {
	    ($dir =~ /\/$/) || ($dir .= "/");
	}
	foreach $file ( readdir(D) ) {
	    next if ( $file  =~ /^\.\.?$/ );
	    $p = $file;
	    ($file =~ /$match/) && (push @files, $p);
	    if ( $descend && -d $p && ! -l $p ) {
		push @files, &find_files($p,$match,$descend);
	    }
	}
	closedir(D);
    }
    return @files;
}


#
# check_unix()
#
# Returns 1 if this is a Unix, 0 otherwise.
#
sub check_unix {
    my($r);
    $r = 0;
    if ( $force_win != 0) {
	return 0;
    }
    if ( -f "/bin/uname" ) {
	$r = 1;
	(-f "\\bin\\uname") && ($r = 0);
    } elsif ( -f "/usr/bin/uname" ) {
        $r = 1;
	(-f "\\usr\\bin\\uname") && ($r = 0);
    }
    if($r) {
	$_ = $Config{'osname'};
	$r = 0 if( /(ms)|(cyg)win/i );
    }
    return $r;
}
__END__
:endofperl
