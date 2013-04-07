#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

package installer::systemactions;

use Cwd;
use File::Copy;
use installer::converter;
use installer::exiter;
use installer::globals;
use installer::pathanalyzer;
use installer::remover;

######################################################
# Creating a new direcotory
######################################################

sub create_directory
{
    my ($directory) = @_;

    create_directory_with_privileges( $directory, "755" );
}

######################################################
# Creating a new direcotory with defined privileges
######################################################

sub create_directory_with_privileges
{
    my ($directory, $privileges) = @_;

    my $returnvalue = 1;
    my $infoline = "";
    my $localprivileges = oct("0".$privileges); # changes "777" to 0777

    if (!(-d $directory))
    {
        $returnvalue = mkdir($directory, $localprivileges);

        if ($returnvalue)
        {
            $infoline = "\nCreated directory: $directory\n";
            push(@installer::globals::logfileinfo, $infoline);

            chmod $localprivileges, $directory;
        }
        else
        {
            # New solution in parallel packing: It is possible, that the directory now exists, although it
            # was not created in this process. There is only an important error, if the directory does not
            # exist now.

            $infoline = "\nDid not succeed in creating directory: \"$directory\". Further attempts will follow.\n";
            push(@installer::globals::logfileinfo, $infoline);

            if (!(-d $directory))
            {
                # Problem with parallel packaging? -> Try a little harder, before exiting.
                # Did someone else remove the parent directory in the meantime?
                my $parentdir = $directory;
                installer::pathanalyzer::get_path_from_fullqualifiedname(\$parentdir);
                if (!(-d $parentdir))
                {
                    $returnvalue = mkdir($directory, $localprivileges);

                    if ($returnvalue)
                    {
                        $infoline = "\nAttention: Successfully created parent directory (should already be created before): $parentdir\n";
                        push(@installer::globals::logfileinfo, $infoline);

                        chmod $localprivileges, $parentdir;
                    }
                    else
                    {
                        $infoline = "\Error: \"$directory\" could not be created. Even the parent directory \"$parentdir\" does not exist and could not be created.\n";
                        push(@installer::globals::logfileinfo, $infoline);
                        if ( -d $parentdir )
                        {
                            $infoline = "\nAttention: Finally the parent directory \"$parentdir\" exists, but I could not create it.\n";
                            push(@installer::globals::logfileinfo, $infoline);
                        }
                        else
                        {
                            # Now it is time to exit, even the parent could not be created.
                            installer::exiter::exit_program("ERROR: Could not create parent directory \"$parentdir\"", "create_directory_with_privileges");
                        }
                    }
                }

                # At this point we have to assume, that the parent directory exist.
                # Trying once more to create the desired directory

                $returnvalue = mkdir($directory, $localprivileges);

                if ($returnvalue)
                {
                    $infoline = "\nAttention: Created directory \"$directory\" in the second try.\n";
                    push(@installer::globals::logfileinfo, $infoline);

                    chmod $localprivileges, $directory;
                }
                else
                {
                    if ( -d $directory )
                    {
                        $infoline = "\nAttention: Finally the directory \"$directory\" exists, but I could not create it.\n";
                        push(@installer::globals::logfileinfo, $infoline);
                    }
                    else
                    {
                        # It is time to exit, even the second try failed.
                        installer::exiter::exit_program("ERROR: Failed to create the directory: $directory", "create_directory_with_privileges");
                    }
                }
            }
            else
            {
                $infoline = "\nAnother process created this directory in exactly this moment :-) : $directory\n";
                push(@installer::globals::logfileinfo, $infoline);
            }
        }
    }
    else
    {
        $infoline = "\nAlready existing directory, did not create: $directory\n";
        push(@installer::globals::logfileinfo, $infoline);

        chmod $localprivileges, $directory;
    }
}

######################################################
# Removing a new direcotory
######################################################

sub remove_empty_directory
{
    my ($directory) = @_;

    my $returnvalue = 1;

    if (-d $directory)
    {
        my $systemcall = "rmdir $directory";

        $returnvalue = system($systemcall);

        my $infoline = "Systemcall: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);

        if ($returnvalue)
        {
            $infoline = "ERROR: Could not remove \"$directory\"!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
        else
        {
            $infoline = "Success: Removed \"$directory\"!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }
}

#######################################################################
# Calculating the number of languages in the string
#######################################################################

sub get_number_of_langs
{
    my ($languagestring) = @_;

    my $number = 1;

    my $workstring = $languagestring;

    while ( $workstring =~ /^\s*(.*)_(.*?)\s*$/ )
    {
        $workstring = $1;
        $number++;
    }

    return $number;
}

#######################################################################
# Creating the directories, in which files are generated or unzipped
#######################################################################

sub create_directories
{
    my ($newdirectory, $languagesref) =@_;

    $installer::globals::unpackpath =~ s/\Q$installer::globals::separator\E\s*$//;  # removing ending slashes and backslashes

    my $path = "";

    if (( $newdirectory eq "uno" ) || ( $newdirectory eq "zip" ) || ( $newdirectory eq "cab" ) || ( $newdirectory =~ /rdb\s*$/i )) # special handling for zip files, cab files and services file because of performance reasons
    {
        if ( $installer::globals::temppathdefined ) { $path = $installer::globals::temppath; }
        else { $path = $installer::globals::unpackpath; }
        $path =~ s/\Q$installer::globals::separator\E\s*$//;    # removing ending slashes and backslashes
        $path = $path . $installer::globals::separator;
    }
    else
    {
        $path = $installer::globals::unpackpath . $installer::globals::separator;

        # special handling, if LOCALINSTALLDIR is set
        if (( $installer::globals::localinstalldirset ) && ( $newdirectory eq "install" ))
        {
            $installer::globals::localinstalldir =~ s/\Q$installer::globals::separator\E\s*$//;
            $path = $installer::globals::localinstalldir . $installer::globals::separator;
        }
    }

    $infoline = "create_directories: Using $path for $newdirectory !\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($newdirectory eq "unzip" )  # special handling for common directory
    {
        $path = $path  . ".." . $installer::globals::separator . "common" . $installer::globals::productextension . $installer::globals::separator;
        create_directory($path);

        $path = $path . $newdirectory . $installer::globals::separator;
        create_directory($path);
    }
    else
    {
        my $localproductname = $installer::globals::product;
        my $localproductsubdir = "";

        if ( $installer::globals::product =~ /^\s*(.+?)\_\_(.+?)\s*$/ )
        {
            $localproductname = $1;
            $localproductsubdir = $2;
        }

        if ( $installer::globals::languagepack ) { $path = $path . $localproductname . "_languagepack" . $installer::globals::separator; }
        elsif ( $installer::globals::helppack ) { $path = $path . $localproductname . "_helppack" . $installer::globals::separator; }
        elsif ( $installer::globals::patch ) { $path = $path . $localproductname . "_patch" . $installer::globals::separator; }
        else { $path = $path . $localproductname . $installer::globals::separator; }

        create_directory($path);

        if ( $localproductsubdir )
        {
            $path = $path . $localproductsubdir . $installer::globals::separator;
            create_directory($path);
        }

        $path = $path . $installer::globals::installertypedir . $installer::globals::separator;
        create_directory($path);

        $path = $path . $newdirectory . $installer::globals::separator;
        create_directory($path);

        my $locallanguagesref = "";

        if ( $$languagesref ) { $locallanguagesref = $$languagesref; }

        if ($newdirectory eq "install" && $installer::globals::ooodownloadfilename ne "" )
        {
            # put packages into versioned path; needed only on linux (fdo#30837)
            $path = $path . "$installer::globals::ooodownloadfilename" . $installer::globals::separator;
            create_directory($path);
        }
        else
        {
            if ($locallanguagesref ne "")   # this will be a path like "01_49", for Profiles and ConfigurationFiles, idt-Files
            {

                my $languagestring = $$languagesref;

                if (length($languagestring) > $installer::globals::max_lang_length )
                {
                    my $number_of_languages = get_number_of_langs($languagestring);
                    chomp(my $shorter = `echo $languagestring | md5sum | sed -e "s/ .*//g"`);
                    my $id = substr($shorter, 0, 8); # taking only the first 8 digits
                    $languagestring = "lang_" . $number_of_languages . "_id_" . $id;
                }

                $path = $path . $languagestring . $installer::globals::separator;
                create_directory($path);
            }
        }
    }

    installer::remover::remove_ending_pathseparator(\$path);

    $path = installer::converter::make_path_conform($path);

    return $path;
}

########################
# Copying one file
########################

sub copy_one_file
{
    my ($source, $dest) = @_;

    my ($returnvalue, $infoline);

    my $copyreturn = copy($source, $dest);

    if ($copyreturn)
    {
        $infoline = "Copy: $source to $dest\n";
        $returnvalue = 1;
    }
    else
    {
        $infoline = "ERROR: Could not copy $source to $dest $!\n";
        $returnvalue = 0;
    }

    push(@installer::globals::logfileinfo, $infoline);

    if ( !$returnvalue ) {
        return $returnvalue;
    }

    # taking care of file attributes
    if ($installer::globals::iswin && -f $dest) {
        my $mode = -x $source ? 0775 : 0664;
        my $mode_str = sprintf("%o", $mode);
        my $chmodreturn = chmod($mode, $dest);
        if ($chmodreturn)
        {
            $infoline = "chmod $mode_str, $dest\n";
            $returnvalue = 1;
        }
        else
        {
            $infoline = "WARNING: Could not chmod $dest: $!\n";
            $returnvalue = 0;
        }

        push(@installer::globals::logfileinfo, $infoline);
    }

    return $returnvalue;
}

##########################
# Hard linking one file
##########################

sub hardlink_one_file
{
    my ($source, $dest) = @_;

    my ($returnvalue, $infoline);

    my $copyreturn = link($source, $dest);

    if ($copyreturn)
    {
        $infoline = "Link: $source to $dest\n";
        $returnvalue = 1;
    }
    else
    {
        $infoline = "ERROR: Could not link $source to $dest\n";
        $returnvalue = 0;
    }

    push(@installer::globals::logfileinfo, $infoline);

    return $returnvalue;
}

##########################
# Soft linking one file
##########################

sub softlink_one_file
{
    my ($source, $dest) = @_;

    my ($returnvalue, $infoline);

    my $linkreturn = symlink($source, $dest);

    if ($linkreturn)
    {
        $infoline = "Symlink: $source to $dest\n";
        $returnvalue = 1;
    }
    else
    {
        $infoline = "ERROR: Could not symlink $source to $dest\n";
        $returnvalue = 0;
    }

    push(@installer::globals::logfileinfo, $infoline);

    return $returnvalue;
}

########################
# Renaming one file
########################

sub rename_one_file
{
    my ($source, $dest) = @_;

    my ($returnvalue, $infoline);

    my $renamereturn = rename($source, $dest);

    if ($renamereturn)
    {
        $infoline = "Rename: $source to $dest\n";
        $returnvalue = 1;
    }
    else
    {
        $infoline = "ERROR: Could not rename $source to $dest\n";
        $returnvalue = 0;
    }

    push(@installer::globals::logfileinfo, $infoline);

    return $returnvalue;
}

##########################################
# Copying all files from one directory
# to another directory
##########################################

sub copy_directory
{
    my ($sourcedir, $destdir) = @_;

    my @sourcefiles = ();

    $sourcedir =~ s/\Q$installer::globals::separator\E\s*$//;
    $destdir =~ s/\Q$installer::globals::separator\E\s*$//;

    my $infoline = "\n";
    push(@installer::globals::logfileinfo, $infoline);
    $infoline = "Copying files from directory $sourcedir to directory $destdir\n";
    push(@installer::globals::logfileinfo, $infoline);

    opendir(DIR, $sourcedir);
    @sourcefiles = readdir(DIR);
    closedir(DIR);

    my $onefile;

    foreach $onefile (@sourcefiles)
    {
        if ((!($onefile eq ".")) && (!($onefile eq "..")))
        {
            my $sourcefile = $sourcedir . $installer::globals::separator . $onefile;
            my $destfile = $destdir . $installer::globals::separator . $onefile;
            if ( -f $sourcefile )   # only files, no directories
            {
                copy_one_file($sourcefile, $destfile);
            }
        }
    }
}

##########################################
# Copying all files from one directory
# to another directory
##########################################

sub is_empty_dir
{
    my ($dir) = @_;

    my $directory_is_empty = 1;
    my @sourcefiles = ();

    opendir(DIR, $dir);
    @sourcefiles = readdir(DIR);
    closedir(DIR);

    my $onefile;
    my @realcontent = ();

    foreach $onefile (@sourcefiles)
    {
        if ((!($onefile eq ".")) && (!($onefile eq "..")))
        {
            push(@realcontent, $onefile);
        }
    }

    if ( $#realcontent > -1 ) { $directory_is_empty = 0; }

    return $directory_is_empty;
}

#####################################################################
# Creating hard links to a complete directory with sub directories.
#####################################################################

sub hardlink_complete_directory
{
    my ($sourcedir, $destdir) = @_;

    my @sourcefiles = ();

    $sourcedir =~ s/\Q$installer::globals::separator\E\s*$//;
    $destdir =~ s/\Q$installer::globals::separator\E\s*$//;

    if ( ! -d $destdir ) { create_directory($destdir); }

    my $infoline = "\n";
    push(@installer::globals::logfileinfo, $infoline);
    $infoline = "Creating hard links for all files from directory $sourcedir to directory $destdir\n";
    push(@installer::globals::logfileinfo, $infoline);

    opendir(DIR, $sourcedir);
    @sourcefiles = readdir(DIR);
    closedir(DIR);

    my $onefile;

    foreach $onefile (@sourcefiles)
    {
        if ((!($onefile eq ".")) && (!($onefile eq "..")))
        {
            my $source = $sourcedir . $installer::globals::separator . $onefile;
            my $dest = $destdir . $installer::globals::separator . $onefile;
            if ( -f $source )   # only files, no directories
            {
                hardlink_one_file($source, $dest);
            }
            if ( -d $source )   # recursive
            {
                hardlink_complete_directory($source, $dest);
            }
        }
    }
}

#####################################################################
# Creating hard links to a complete directory with sub directories.
#####################################################################

sub softlink_complete_directory
{
    my ($sourcedir, $destdir, $depth) = @_;

    my @sourcefiles = ();

    $sourcedir =~ s/\Q$installer::globals::separator\E\s*$//;
    $destdir =~ s/\Q$installer::globals::separator\E\s*$//;

    if ( ! -d $destdir ) { create_directory($destdir); }

    my $infoline = "\n";
    push(@installer::globals::logfileinfo, $infoline);
    $infoline = "Creating soft links for all files from directory $sourcedir to directory $destdir\n";
    push(@installer::globals::logfileinfo, $infoline);

    opendir(DIR, $sourcedir);
    @sourcefiles = readdir(DIR);
    closedir(DIR);

    my $onefile;

    foreach $onefile (@sourcefiles)
    {
        if ((!($onefile eq ".")) && (!($onefile eq "..")))
        {
            my $source = $sourcedir . $installer::globals::separator . $onefile;
            my $dest = $destdir . $installer::globals::separator . $onefile;
            if ( -f $source )   # only files, no directories
            {
                my $localsource = $source;
                if ( $depth > 0 ) { for ( my $i = 1; $i <= $depth; $i++ ) { $localsource = "../" . $localsource; } }
                softlink_one_file($localsource, $dest);
            }
            if ( -d $source )   # recursive
            {
                my $newdepth = $depth + 1;
                softlink_complete_directory($source, $dest, $newdepth);
            }
        }
    }
}

#####################################################
# Copying a complete directory with sub directories.
#####################################################

sub copy_complete_directory
{
    my ($sourcedir, $destdir) = @_;

    my @sourcefiles = ();

    $sourcedir =~ s/\Q$installer::globals::separator\E\s*$//;
    $destdir =~ s/\Q$installer::globals::separator\E\s*$//;

    if ( ! -d $destdir ) { create_directory($destdir); }

    my $infoline = "\n";
    push(@installer::globals::logfileinfo, $infoline);
    $infoline = "Copying files from directory $sourcedir to directory $destdir\n";
    push(@installer::globals::logfileinfo, $infoline);

    opendir(DIR, $sourcedir);
    @sourcefiles = readdir(DIR);
    closedir(DIR);

    my $onefile;

    foreach $onefile (@sourcefiles)
    {
        if ((!($onefile eq ".")) && (!($onefile eq "..")))
        {
            my $source = $sourcedir . $installer::globals::separator . $onefile;
            my $dest = $destdir . $installer::globals::separator . $onefile;
            if ( -f $source )   # only files, no directories
            {
                copy_one_file($source, $dest);
            }
            if ( -d $source )   # recursive
            {
                if ((!( $source =~ /packages\/SUNW/ )) && (!( $source =~ /packages\/OOO/ )))    # do not copy complete Solaris packages!
                {
                    copy_complete_directory($source, $dest);
                }
            }
        }
    }
}

#####################################################################################
# Copying a complete directory with sub directories, but not the CVS directories.
#####################################################################################

sub copy_complete_directory_without_cvs
{
    my ($sourcedir, $destdir) = @_;

    my @sourcefiles = ();

    $sourcedir =~ s/\Q$installer::globals::separator\E\s*$//;
    $destdir =~ s/\Q$installer::globals::separator\E\s*$//;

    if ( ! -d $destdir ) { create_directory($destdir); }

    my $infoline = "\n";
    push(@installer::globals::logfileinfo, $infoline);
    $infoline = "Copying files from directory $sourcedir to directory $destdir (without CVS)\n";
    push(@installer::globals::logfileinfo, $infoline);

    opendir(DIR, $sourcedir);
    @sourcefiles = readdir(DIR);
    closedir(DIR);

    my $onefile;

    foreach $onefile (@sourcefiles)
    {
        if ((!($onefile eq ".")) && (!($onefile eq "..")) && (!($onefile eq "CVS")))
        {
            my $source = $sourcedir . $installer::globals::separator . $onefile;
            my $dest = $destdir . $installer::globals::separator . $onefile;
            if ( -f $source )   # only files, no directories
            {
                copy_one_file($source, $dest);
            }
            if ( -d $source )   # recursive
            {
                copy_complete_directory_without_cvs($source, $dest);
            }
        }
    }
}

#####################################################
# Copying all files with a specified file extension
# from one directory to another directory.
#####################################################

sub copy_directory_with_fileextension
{
    my ($sourcedir, $destdir, $extension) = @_;

    my @sourcefiles = ();

    $sourcedir =~ s/\Q$installer::globals::separator\E\s*$//;
    $destdir =~ s/\Q$installer::globals::separator\E\s*$//;

    $infoline = "\n";
    push(@installer::globals::logfileinfo, $infoline);
    $infoline = "Copying files with extension $extension from directory $sourcedir to directory $destdir\n";
    push(@installer::globals::logfileinfo, $infoline);

    opendir(DIR, $sourcedir);
    @sourcefiles = readdir(DIR);
    closedir(DIR);

    my $onefile;

    foreach $onefile (@sourcefiles)
    {
        if ((!($onefile eq ".")) && (!($onefile eq "..")))
        {
            if ( $onefile =~ /\.$extension\s*$/ )   # only copying specified files
            {
                my $sourcefile = $sourcedir . $installer::globals::separator . $onefile;
                my $destfile = $destdir . $installer::globals::separator . $onefile;
                if ( -f $sourcefile )   # only files, no directories
                {
                    copy_one_file($sourcefile, $destfile);
                }
            }
        }
    }
}

#########################################################
# Copying all files without a specified file extension
# from one directory to another directory.
#########################################################

sub copy_directory_except_fileextension
{
    my ($sourcedir, $destdir, $extension) = @_;

    my @sourcefiles = ();

    $sourcedir =~ s/\Q$installer::globals::separator\E\s*$//;
    $destdir =~ s/\Q$installer::globals::separator\E\s*$//;

    $infoline = "\n";
    push(@installer::globals::logfileinfo, $infoline);
    $infoline = "Copying files without extension $extension from directory $sourcedir to directory $destdir\n";
    push(@installer::globals::logfileinfo, $infoline);

    opendir(DIR, $sourcedir);
    @sourcefiles = readdir(DIR);
    closedir(DIR);

    my $onefile;

    foreach $onefile (@sourcefiles)
    {
        if ((!($onefile eq ".")) && (!($onefile eq "..")))
        {
            if ( ! ( $onefile =~ /\.$extension\s*$/ ))  # only copying not having the specified extension
            {
                my $sourcefile = $sourcedir . $installer::globals::separator . $onefile;
                my $destfile = $destdir . $installer::globals::separator . $onefile;
                if ( -f $sourcefile )   # only files, no directories
                {
                    copy_one_file($sourcefile, $destfile);
                }
            }
        }
    }
}

########################################################
# Renaming all files with a specified file extension
# in a specified directory.
# Example: "Feature.idt.01" -> "Feature.idt"
########################################################

sub rename_files_with_fileextension
{
    my ($dir, $extension) = @_;

    my @sourcefiles = ();

    $dir =~ s/\Q$installer::globals::separator\E\s*$//;

    my $infoline = "\n";
    push(@installer::globals::logfileinfo, $infoline);
    $infoline = "Renaming files with extension \"$extension\" in the directory $dir\n";
    push(@installer::globals::logfileinfo, $infoline);

    opendir(DIR, $dir);
    @sourcefiles = readdir(DIR);
    closedir(DIR);

    my $onefile;

    foreach $onefile (@sourcefiles)
    {
        if ((!($onefile eq ".")) && (!($onefile eq "..")))
        {
            if ( $onefile =~ /^\s*(\S.*?)\.$extension\s*$/ )    # only renaming specified files
            {
                my $destfile = $1;
                my $sourcefile = $dir . $installer::globals::separator . $onefile;
                $destfile = $dir . $installer::globals::separator . $destfile;
                if ( -f $sourcefile )   # only files, no directories
                {
                    rename_one_file($sourcefile, $destfile);
                }
            }
        }
    }
}

########################################################
# Finding all files with a specified file extension
# in a specified directory.
########################################################

sub find_file_with_file_extension
{
    my ($extension, $dir) = @_;

    my @allfiles = ();

    $dir =~ s/\Q$installer::globals::separator\E\s*$//;

    my $infoline = "\n";
    push(@installer::globals::logfileinfo, $infoline);
    $infoline = "Searching files with extension \"$extension\" in the directory $dir\n";
    push(@installer::globals::logfileinfo, $infoline);

    opendir(DIR, $dir);
    @sourcefiles = sort readdir(DIR);
    closedir(DIR);

    my $onefile;

    foreach $onefile (@sourcefiles)
    {
        if ((!($onefile eq ".")) && (!($onefile eq "..")))
        {
            if ( $onefile =~ /^\s*(\S.*?)\.$extension\s*$/ )
            {
                push(@allfiles, $onefile)
            }
        }
    }

    return \@allfiles;
}

##############################################################
# Creating a unique directory, for example "01_inprogress_7"
# in the install directory.
##############################################################

sub make_numbered_dir
{
    my ($newstring, $olddir) = @_;

    my $basedir = $olddir;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$basedir);

    my $alldirs = get_all_directories($basedir);

    # searching for the highest number extension

    my $maxnumber = 0;

    for ( my $i = 0; $i <= $#{$alldirs}; $i++ )
    {
        if ( ${$alldirs}[$i] =~ /\_(\d+)\s*$/ )
        {
            my $number = $1;
            if ( $number > $maxnumber ) { $maxnumber = $number; }
        }
    }

    my $newnumber = $maxnumber + 1;

    my $newdir = $olddir . "_" . $newstring . "_" . $newnumber;

    my $returndir = "";

    if ( move($olddir, $newdir) )
    {
        $infoline = "\nMoved directory from $olddir to $newdir\n";
        push(@installer::globals::logfileinfo, $infoline);
        $returndir = $newdir;
    }
    else
    {
        $infoline = "\nATTENTION: Could not move directory from $olddir to $newdir, \"make_numbered_dir\"\n";
        push(@installer::globals::logfileinfo, $infoline);
        $returndir = $olddir;
    }

    return $returndir;
}

##############################################################
# Determining the highest number in the install directory.
##############################################################

sub determine_maximum_number
{
    my ($dir, $languagestringref) = @_;

    my $basedir = $dir;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$basedir);

    my $alldirs = get_all_directories($basedir);

    my $maxnumber = 1;

    # In control.pm the installation directory is determined as:
    # $installer::globals::build . "_" . $installer::globals::lastminor . "_" .
    # "native_inprogress-number_" . $$languagesref . "\." . $installer::globals::buildid;

    # searching for the highest number extension after the first "-", which belongs to
    # $installer::globals::build, $installer::globals::lastminor and $installer::globals::buildid
    # In this step not looking for the language!

    my @correctbuildiddirs = ();

    for ( my $i = 0; $i <= $#{$alldirs}; $i++ )
    {
        my $onedir = ${$alldirs}[$i];
        installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$onedir);

        if ( $onedir =~ /^\s*\Q$installer::globals::build\E\_\Q$installer::globals::lastminor\E\_(.*?)\-(\d+)\_(.*?)\.\Q$installer::globals::buildid\E\s*$/ )
        {
            my $number = $2;
            if ( $number > $maxnumber ) { $maxnumber = $number; }
            push(@correctbuildiddirs, $onedir);
        }
    }

    # From all directories with correct $installer::globals::build, $installer::globals::lastminor
    # and $installer::globals::buildid, those directories, which already have the maximum number
    # have to be selected

    my @maximumnumberdirs = ();

    for ( my $i = 0; $i <= $#correctbuildiddirs; $i++ )
    {
        my $onedir = $correctbuildiddirs[$i];

        if ( $onedir =~ /^\s*(.*?)\-(\d+)\_(.*?)\.(.*?)\s*$/ )
        {
            my $number = $2;

            if ( $number == $maxnumber )
            {
                push(@maximumnumberdirs, $onedir);
            }
        }
    }

    # @maximumnumberdirs contains only those directories with correct $installer::globals::build,
    # $installer::globals::lastminor and $installer::globals::buildid, which already have the maximum number.
    # If the current language is part of this directory, the number has to be increased.

    my $increase_counter = 0;

    for ( my $i = 0; $i <= $#maximumnumberdirs; $i++ )
    {
        my $onedir = $maximumnumberdirs[$i];

        if ( $onedir =~ /^\s*(.*?)\-(\d+)\_(.*?)\.(.*?)\s*$/ )
        {
            my $number = $2;
            my $languagestring = $3;

            if ( $languagestring eq $$languagestringref )
            {
                $increase_counter = 1;
            }
        }
    }

    if ( $increase_counter )
    {
        $maxnumber = $maxnumber + 1;
    }

    return $maxnumber;
}

#####################################################################################
# Renaming a directory by exchanging a string, for example from "01_inprogress_7"
# to "01_witherror_7".
#####################################################################################

sub rename_string_in_directory
{
    my ($olddir, $oldstring, $newstring) = @_;

    my $newdir = $olddir;
    my $infoline = "";

    $newdir =~ s/$oldstring/$newstring/g;

    if (( -d $newdir ) && ( $olddir ne $newdir )) { remove_complete_directory($newdir, 1); }

    if ( move($olddir, $newdir) )
    {
        $infoline = "\nMoved directory from $olddir to $newdir\n";
        push(@installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "\nATTENTION: Could not move directory from $olddir to $newdir, \"rename_string_in_directory\"\n";
        push(@installer::globals::logfileinfo, $infoline);
    }

    return $newdir;
}

######################################################
# Returning the complete directory name,
# input is the first part of the directory name.
######################################################

sub get_directoryname
{
    my ($searchdir, $startstring) = @_;

    my $dirname = "";
    my $founddir = 0;
    my $direntry;

    opendir(DIR, $searchdir);

    foreach $direntry (readdir (DIR))
    {
        next if $direntry eq ".";
        next if $direntry eq "..";

        if (( -d $direntry ) && ( $direntry =~ /^\s*\Q$startstring\E/ ))
        {
            $dirname = $direntry;
            $founddir = 1;
            last;
        }
    }

    closedir(DIR);

    if ( ! $founddir ) { installer::exiter::exit_program("ERROR: Did not find directory beginning with $startstring in directory $searchdir", "get_directoryname"); }

    return $dirname;
}


###################################
# Renaming a directory
###################################

sub rename_directory
{
    my ($olddir, $newdir) = @_;

    my $infoline = "";

    if ( move($olddir, $newdir) )
    {
        $infoline = "\nMoved directory from $olddir to $newdir\n";
        push(@installer::globals::logfileinfo, $infoline);
    }
    else
    {
        installer::exiter::exit_program("ERROR: Could not move directory from $olddir to $newdir $!", "rename_directory");
    }

    return $newdir;
}

##############################################################
# Creating a directory next to an existing directory
##############################################################

sub create_directory_next_to_directory
{
    my ($topdir, $dirname) = @_;

    my $basedir = $topdir;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$basedir);

    $basedir =~ s/\Q$installer::globals::separator\E\s*$//;

    my $newdir = $basedir . $installer::globals::separator . $dirname;

    create_directory($newdir);

    return $newdir;
}

##############################################################
# Collecting all directories inside a directory
##############################################################

sub get_all_directories
{
    my ($basedir) = @_;

    my @alldirs = ();
    my $direntry;

    $basedir =~ s/\Q$installer::globals::separator\E\s*$//;

    opendir(DIR, $basedir);

    foreach $direntry (readdir (DIR))
    {
        next if $direntry eq ".";
        next if $direntry eq "..";

        my $completeentry = $basedir . $installer::globals::separator . $direntry;

        if ( -d $completeentry ) { push(@alldirs, $completeentry); }
    }

    closedir(DIR);

    return \@alldirs;
}

##############################################################
# Collecting all directories inside a directory
# Returning without path
##############################################################

sub get_all_directories_without_path
{
    my ($basedir) = @_;

    my @alldirs = ();
    my $direntry;

    $basedir =~ s/\Q$installer::globals::separator\E\s*$//;

    opendir(DIR, $basedir);

    foreach $direntry (readdir (DIR))
    {
        next if $direntry eq ".";
        next if $direntry eq "..";

        my $completeentry = $basedir . $installer::globals::separator . $direntry;

        if ( -d $completeentry ) { push(@alldirs, $direntry); }
    }

    closedir(DIR);

    return \@alldirs;
}

##############################################################
# Collecting all files inside one directory
##############################################################

sub get_all_files_from_one_directory
{
    my ($basedir) = @_;

    my @allfiles = ();
    my $direntry;

    $basedir =~ s/\Q$installer::globals::separator\E\s*$//;

    opendir(DIR, $basedir);

    foreach $direntry (readdir (DIR))
    {
        next if $direntry eq ".";
        next if $direntry eq "..";

        my $completeentry = $basedir . $installer::globals::separator . $direntry;

        if ( -f $completeentry ) { push(@allfiles, $completeentry); }
    }

    closedir(DIR);

    return \@allfiles;
}

##############################################################
# Collecting all files inside one directory
##############################################################

sub get_all_files_from_one_directory_without_path
{
    my ($basedir) = @_;

    my @allfiles = ();
    my $direntry;

    $basedir =~ s/\Q$installer::globals::separator\E\s*$//;

    opendir(DIR, $basedir);

    foreach $direntry (readdir (DIR))
    {
        next if $direntry eq ".";
        next if $direntry eq "..";

        my $completeentry = $basedir . $installer::globals::separator . $direntry;

        if ( -f $completeentry ) { push(@allfiles, $direntry); }
    }

    closedir(DIR);

    return \@allfiles;
}

##############################################################
# Collecting all files and directories inside one directory
##############################################################

sub read_directory
{
    my ($basedir) = @_;

    my @allcontent = ();
    my $direntry;

    $basedir =~ s/\Q$installer::globals::separator\E\s*$//;

    opendir(DIR, $basedir);

    foreach $direntry (readdir (DIR))
    {
        next if $direntry eq ".";
        next if $direntry eq "..";

        my $completeentry = $basedir . $installer::globals::separator . $direntry;

        if (( -f $completeentry ) || ( -d $completeentry )) { push(@allcontent, $completeentry); }
    }

    closedir(DIR);

    return \@allcontent;
}

##############################################################
# Finding the new content in a directory
##############################################################

sub find_new_content_in_directory
{
    my ( $basedir, $oldcontent ) = @_;

    my @newcontent = ();
    my @allcontent = ();

    my $direntry;

    $basedir =~ s/\Q$installer::globals::separator\E\s*$//;

    opendir(DIR, $basedir);

    foreach $direntry (readdir (DIR))
    {
        next if $direntry eq ".";
        next if $direntry eq "..";

        my $completeentry = $basedir . $installer::globals::separator . $direntry;

        if (( -f $completeentry ) || ( -d $completeentry ))
        {
            push(@allcontent, $completeentry);
            if (! installer::existence::exists_in_array($completeentry, $oldcontent))
            {
                push(@newcontent, $completeentry);
            }
        }
    }

    closedir(DIR);

    return (\@newcontent, \@allcontent);
}

##############################################################
# Trying to create a directory, no error if this fails
##############################################################

sub try_to_create_directory
{
    my ($directory) = @_;

    my $returnvalue = 1;
    my $created_directory = 0;

    if (!(-d $directory))
    {
        $returnvalue = mkdir($directory, 0775);

        if ($returnvalue)
        {
            $created_directory = 1;
            $infoline = "\nCreated directory: $directory\n";
            push(@installer::globals::logfileinfo, $infoline);

            chmod 0775, $directory;
        }
        else
        {
            $created_directory = 0;
        }
    }
    else
    {
        $created_directory = 1;
    }

    return $created_directory;
}

##############################################################
# Creating a complete directory structure
##############################################################

sub create_directory_structure
{
    my ($directory) = @_;

    if ( ! try_to_create_directory($directory) )
    {
        my $parentdir = $directory;
        installer::pathanalyzer::get_path_from_fullqualifiedname(\$parentdir);

        my $infoline = "INFO: Did not create directory $directory\n";
        push(@installer::globals::logfileinfo, $infoline);
        $infoline = "Now trying to create parent directory $parentdir\n";
        push(@installer::globals::logfileinfo, $infoline);

        create_directory_structure($parentdir);                                 # recursive
    }

    create_directory($directory);   # now it has to succeed
}

######################################################
# Removing a complete directory with subdirectories
######################################################

sub remove_complete_directory
{
    my ($directory, $start) = @_;

    my @content = ();
    my $infoline = "";

    $directory =~ s/\Q$installer::globals::separator\E\s*$//;

    if ( -d $directory )
    {
        if ( $start )
        {
            $infoline = "\n";
            push(@installer::globals::logfileinfo, $infoline);
            $infoline = "Removing directory $directory\n";
            push(@installer::globals::logfileinfo, $infoline);
        }

        opendir(DIR, $directory);
        @content = readdir(DIR);
        closedir(DIR);

        my $oneitem;

        foreach $oneitem (@content)
        {
            if ((!($oneitem eq ".")) && (!($oneitem eq "..")))
            {
                my $item = $directory . $installer::globals::separator . $oneitem;

                if ( -f $item || -l $item )     # deleting files or links
                {
                    unlink($item);
                }

                if ( -d $item )     # recursive
                {
                    remove_complete_directory($item, 0);
                }
            }
        }

        # try to remove empty directory

        my $returnvalue = rmdir $directory;

        if ( ! $returnvalue )
        {
            $infoline = "Warning: Problem with removing empty dir $directory\n";
            push(@installer::globals::logfileinfo, $infoline);
        }

        # try a little bit harder (sometimes there is a performance problem)
        if ( -d $directory )
        {
            for ( my $j = 1; $j <= 3; $j++ )
            {
                if ( -d $directory )
                {
                    $infoline = "\n";
                    push(@installer::globals::logfileinfo, $infoline);
                    $infoline = "Warning (Try $j): Problems with removing directory $directory\n";
                    push(@installer::globals::logfileinfo, $infoline);

                    $returnvalue = rmdir $directory;

                    if ( $returnvalue )
                    {
                        $infoline = "Successfully removed empty dir $directory\n";
                        push(@installer::globals::logfileinfo, $infoline);
                    } else {
                        $infoline = "Warning: rmdir $directory failed.\n";
                        push(@installer::globals::logfileinfo, $infoline);
                    }
                }
            }
        }
    }
}

######################################################
# Creating a unique directory with number extension
######################################################

sub create_unique_directory
{
    my ($directory) = @_;

    $directory =~ s/\Q$installer::globals::separator\E\s*$//;
    $directory = $directory . "_INCREASINGNUMBER";

    my $counter = 1;
    my $created = 0;
    my $localdirectory = "";

    do
    {
        $localdirectory = $directory;
        $localdirectory =~ s/INCREASINGNUMBER/$counter/;
        $counter++;

        if ( ! -d $localdirectory )
        {
            create_directory($localdirectory);
            $created = 1;
        }
    }
    while ( ! $created );

    return $localdirectory;
}

######################################################
# Creating a unique directory with pid extension
######################################################

sub create_pid_directory
{
    my ($directory) = @_;

    $directory =~ s/\Q$installer::globals::separator\E\s*$//;
    my $pid = $$;           # process id
    my $time = time();      # time

    $directory = $directory . "_" . $pid . $time;

    if ( ! -d $directory ) { create_directory($directory); }
    else { installer::exiter::exit_program("ERROR: Directory $directory already exists!", "create_pid_directory"); }

    return $directory;
}

##############################################################
# Reading all files from a directory and its subdirectories
##############################################################

sub read_complete_directory
{
    my ($directory, $pathstring, $filecollector) = @_;

    my @content = ();
    opendir(DIR, $directory);
    @content = readdir(DIR);
    closedir(DIR);

    my $onefile;

    foreach $onefile (@content)
    {
        if ((!($onefile eq ".")) && (!($onefile eq "..")))
        {
            my $completefilename = $directory . $installer::globals::separator . $onefile;
            my $sep = "";
            if ( $pathstring ne "" ) { $sep = $installer::globals::separator; }

            if ( ! -d $completefilename )   # only files, no directories
            {
                my $content = $pathstring . $sep . $onefile;
                push(@{$filecollector}, $content);
            }
            else  # recursive for directories
            {
                my $newpathstring = $pathstring . $sep . $onefile;
                read_complete_directory($completefilename, $newpathstring, $filecollector);
            }
        }
    }
}

##############################################################
# Reading all files from a directory and its subdirectories
# Version 2
##############################################################

sub read_full_directory {
    my ( $currentdir, $pathstring, $collector ) = @_;
    my $item;
    my $fullname;
    local *DH;

    unless (opendir(DH, $currentdir))
    {
        return;
    }
    while (defined ($item = readdir(DH)))
    {
        next if($item eq "." or $item eq "..");
        $fullname = $currentdir . $installer::globals::separator . $item;
        my $sep = "";
        if ( $pathstring ne "" ) { $sep = $installer::globals::separator; }

        if( -d $fullname)
        {
            my $newpathstring = $pathstring . $sep . $item;
            read_full_directory($fullname, $newpathstring, $collector) if(-d $fullname);
        }
        else
        {
            my $content = $pathstring . $sep . $item;
            push(@{$collector}, $content);
        }
    }
    closedir(DH);
    return
}

##############################################################
# Removing all empty directories below a specified directory
##############################################################

sub remove_empty_dirs_in_folder
{
    my ( $dir ) = @_;

    my @content = ();
    my $infoline = "";

    $dir =~ s/\Q$installer::globals::separator\E\s*$//;

    if ( -d $dir )
    {
        opendir(DIR, $dir);
        @content = readdir(DIR);
        closedir(DIR);

        my $oneitem;

        foreach $oneitem (@content)
        {
            if ((!($oneitem eq ".")) && (!($oneitem eq "..")))
            {
                my $item = $dir . $installer::globals::separator . $oneitem;

                if ( -d $item ) # recursive
                {
                    remove_empty_dirs_in_folder($item);
                }
            }
        }

        # try to remove empty directory
        my $returnvalue = rmdir $dir;

        if ( $returnvalue )
        {
            $infoline = "Successfully removed empty dir $dir\n";
            push(@installer::globals::logfileinfo, $infoline);
        }

    }

}

1;
