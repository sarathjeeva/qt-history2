#!/usr/bin/perl -w

use strict;

($#ARGV == -1) && die "$0 directory <export_symbol>\n";
my $EXPORT_DIRECTORY = $ARGV[0];
my $EXPORT_SYMBOL = "Q_[^ ]*_EXPORT(?:_[^ ]*)?";
$EXPORT_SYMBOL = $ARGV[1] if($#ARGV == 1);

my %CLASSES=();
my %GLOBALS=();

sub find_classnames {
    my $ret = 0;
    my ($file) = @_;

    my $parsable = "";
    if(open(F, "<$file")) {
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
	if($definition && $definition =~ /$EXPORT_SYMBOL /) {
	    if($definition =~ m/^ *typedef/) {
	    } elsif($definition =~ m/^ *(template<.*> *)?(class|struct) +$EXPORT_SYMBOL +([^ ]+) ?((,|:) *(public|private) *.*)? *\{\}$/) {
		$CLASSES{$3} = "$file" unless(defined $CLASSES{$3});
	    } elsif($definition =~ /$EXPORT_SYMBOL +([a-zA-Z0-9_\*& ]* +)?[&\*]?([a-zA-Z0-9>=!\*<+_-]*)( *\(.*\))? *(;|\{\})$/) {
		$GLOBALS{$2} = "$file" unless(defined $GLOBALS{$2});
	    }
	}
    }
    return $ret;
}

#find the symbols
sub find_files {
    my ($dir) = @_;
    ($dir = ".") if($dir eq "");

    local(*D);
    if ( opendir(D,$dir) ) {
	if ($dir eq ".") {
	    $dir = "";
	} else {
	    ($dir =~ /\/$/) || ($dir .= "/");
	}
	foreach (readdir(D) ) {
	    next if ( /^\.\.?$/ );
	    my $p = "$dir/$_";
	    if(-d "$p") {
		find_files("$p");
	    } elsif($p =~ /\.(h|cpp)$/) {
		find_classnames("$p");
	    }
	}
	closedir(D);
    }
}
find_files("$EXPORT_DIRECTORY");

#generate output
print "{\n";
print "  global:\n";
print "  extern \"C++\"\n";
print "  {";
my $symbol_count = 0;
foreach (keys %CLASSES) {
     my @symbols = ("${_}::*", "${_}?virtual?table", "${_}?type_info?node", "${_}?type_info?function",
		    "vtable?for?${_}", "typeinfo?for?${_}", "non-virtual?thunk?to?${_}::*");
     foreach (@symbols) {
	 print ";" if($symbol_count);
	 print "\n     ${_}";
	 $symbol_count++;
     }
}
foreach (keys %GLOBALS) {
    print ";" if($symbol_count);
    print "\n     ${_}*";
    $symbol_count++;
}
print "\n  };\n";
print "  local:\n";
print "  extern \"C++\"\n";
print "  {\n";
print "    *\n";
print "  };\n";
print "};\n";
