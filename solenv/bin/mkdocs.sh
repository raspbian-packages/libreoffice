#!/usr/bin/env bash
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Doxygen / README doc generation
#
# See git for contributors
#

function header {
  title=$1
  breadcrumb=$2
  output=$3

  cat - > $output <<EOF
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<html>
<head>
	<title>$title</title>

	<style>
	* { margin: 0; padding: 0; }
	body { font-family: sans-serif; font-size: 12px; }
	#head { padding: 20px; background: #18A303; }
	#head a { color: #000; }
	#body { padding: 20px; }
	#foot { padding: 10px; font-size: 9px; border-top: 1px #18A303 solid; margin-top: 25px; }
	p { line-height: 1.7em; margin-bottom: 1em; }
	pre { margin-bottom: 0.5em; }
	.multi-col { -moz-column-width: 13em; -webkit-column-width: 13em; -moz-column-gap: 1em; -webkit-column-gap: 1em; }
	h1 {margin-bottom: 0.5em;}
	h2,h3,h4 { margin: 1.3em 0 0.5em 0; }
	ul, ol { margin: 0.5em 1.5em; }
	</style>
</head>
<body>
	<div id="head">
	<h1>$title</h1>
	<p>$breadcrumb</p>
	</div>
	<div id="body">
EOF
}

function footer {
  output=$1

  cat - >> $output <<EOF

</div>
<div id="foot">
  <small>
    <p>Generated by Libreoffice <a href="http://cgit.freedesktop.org/libreoffice/core/plain/solenv/bin/mkdocs.sh">Module Description Tool</a></p>
    <p>Last updated:
EOF
LANG= date >> $output
cat - >> $output <<EOF
    </p>
  </small>
</div>
</body>
</html>
EOF

}

function proc_text {
  # Local links: [[...]]
  # Git links: [git:...]
  # Other remote links: [...]
  # Headings: == bleh ==
  # Paragraphs: \n\n
  sed -re ' s/\[\[([-_a-zA-Z0-9]+)\]\]/<a href="\1.html">\1<\/a>/g' - \
  | sed -re ' s/\[git:([^]]+)\]/<a href="http:\/\/cgit.freedesktop.org\/libreoffice\/core\/tree\/\1">\1<\/a>/g' \
  | sed -re ' s/\[([^]]+)\]/<a href="\1">\1<\/a>/g' \
  | sed -re ' s/====([^=]+)====/<h4>\1<\/h4>/g' \
  | sed -re ' s/===([^=]+)===/<h3>\1<\/h3>/g' \
  | sed -re ' s/==([^=]+)==/<h2>\1<\/h2>/g' \
  | sed -re ':a;N;$!ba;s/\n\n/<\/p><p>/g' \
  | awk 'BEGIN { print "<p>" } { print } END { print "</p>" }'
}

function setup {
  parm=$1
  if [ -z "${!parm}" ] ; then
	echo "grep \"${parm}=\" ./config_host.mk | sed -re \" s/${parm}=//\")"  
    echo "$parm=$(grep \"${parm}=\" ./config_host.mk | sed -re \" s/${parm}=//\")"
    eval "$parm=$(grep \"${parm}=\" ./config_host.mk | sed -re \" s/${parm}=//\")"
  fi
  if [ -z "${!parm}" ] ; then
    echo "could not determine $parm" >&2
    exit 1
  fi
}

# binaries that we need
which doxygen > /dev/null 2>&1 || {
    echo "You need doxygen for doc generation" >&2
    exit 1
}
which dot > /dev/null 2>&1 || {
    echo "You need the graphviz tools to create the nice inheritance graphs" >&2
    exit 1
}

# suck setup
setup "SOLARINC"
shopt -s nullglob

# Title of the documentation
DOXYGEN_PROJECT_PREFIX="LibreOffice"

# get list of modules
INPUT_PROJECTS="`ls */Module_*.mk | sed 's#/.*##'`"

# output directory for generated documentation
BASE_OUTPUT="$1"
mkdir -p "$BASE_OUTPUT" || {
    echo "Cannot create $BASE_OUTPUT"
    exit 1
}

# paths for binary and configuration file
BASE_PATH=`pwd`
DOXYGEN_CFG="$2"
if test ! -f "$DOXYGEN_CFG"; then
    echo "doxygen.cfg not found"
    exit 1
fi

# strip -I. and bin -I prefix; exlude system headers
DOXYGEN_INCLUDE_PATH=`echo $SOLARINC | sed -e 's/-I\.//g' -e 's/ -I/ /'g -e 's/ -isystem/ /g' -e 's|/usr/[^ ]*| |g'`

# setup version string
DOXYGEN_VERSION="master"


###################################################
#
# Generate docs
#
###################################################

# cleanup
echo "cleaning up" && rm -rf $BASE_OUTPUT/*

# make the stuff world-readable
umask 022

# generate docs
echo "generating doxygen docs"
DOXYGEN_REF_TAGFILES=""
for PROJECT in $INPUT_PROJECTS;
do
  # avoid processing of full project subdirs, only add source and inc

  # project header files can be in $PROJECT/inc and/pr include/$PROJECT
  if [ -d "$PROJECT/inc" ]; then
      PROJECT_INCLUDE="$PROJECT/inc"
  fi

  if [ -d "include/$PROJECT" ]; then
      PROJECT_INCLUDE="$PROJECT_INCLUDE include/$PROJECT"
  fi

  DOXYGEN_INPUT=`printf "%s" "$PROJECT/source $PROJECT_INCLUDE"`

  DOXYGEN_OUTPUT="$BASE_OUTPUT/$PROJECT"
  DOXYGEN_OUR_TAGFILE="$DOXYGEN_OUTPUT/$PROJECT.tags"
  DOXYGEN_PROJECTNAME="$DOXYGEN_PROJECT_PREFIX Module $PROJECT"

  # export variables referenced in doxygen config file
  export DOXYGEN_INPUT
  export DOXYGEN_OUTPUT
  export DOXYGEN_INCLUDE_PATH
  export DOXYGEN_VERSION
  export DOXYGEN_OUR_TAGFILE
  export DOXYGEN_REF_TAGFILES
  export DOXYGEN_PROJECTNAME

  # debug
  echo "Calling $DOXYGEN_PATH/doxygen $DOXYGEN_CFG with"
  echo "Input:      $DOXYGEN_INPUT"
  echo "Output:     $DOXYGEN_OUTPUT"
  echo "Include:    $DOXYGEN_INCLUDE_PATH"
  echo "Version:    $DOXYGEN_VERSION"
  echo "Tagfile:    $DOXYGEN_OUR_TAGFILE"
  echo "Ref-Tags:   $DOXYGEN_REF_TAGFILES"
  echo "Title:      $DOXYGEN_PROJECTNAME"

  nice -15 doxygen "$DOXYGEN_CFG" >>$BASE_OUTPUT/doxygen.log || exit 1

  # setup referenced tagfiles for next round
  DOXYGEN_REF_TAGFILES="$DOXYGEN_REF_TAGFILES $DOXYGEN_OUR_TAGFILE=$BASE_URL/$PROJECT/html"
done

# generate entry page
echo "generating index page"
header "LibreOffice Modules" " " "$BASE_OUTPUT/index.html"
for module_name in *; do
  if [ -d $module_name ]; then
    cur_file=$(echo $module_name/README* $module_name/readme.txt*)
	if [ -f "$cur_file" ]; then
      # write index.html entry
      text="<h2><a href=\"${module_name}.html\">${module_name}</a></h2>\n"
	  text="${text}$(head -n1 $cur_file | proc_text )"
	  echo -e $text >> "$BASE_OUTPUT/index.html"

      # write detailed module content
      header "$module_name" "<a href=\"index.html\">LibreOffice</a> &raquo; ${module_name}" "$BASE_OUTPUT/${module_name}.html"
      text="<p><b>View module in:</b>"
	  text="${text} &nbsp; <a href=\"http://cgit.freedesktop.org/libreoffice/core/tree/${module_name}\">cgit</a>"
      if $(echo $INPUT_PROJECTS | grep -q $module_name); then
        text="${text} &nbsp; <a href=\"${module_name}/html/classes.html\">Doxygen</a>"
      fi
      text="${text} </p><p>&nbsp;</p>"
	  echo -e $text >> "$BASE_OUTPUT/${module_name}.html"
	  proc_text < $cur_file >> "$BASE_OUTPUT/${module_name}.html"
	  footer "$BASE_OUTPUT/${module_name}.html"
    else
      empty_modules[${#empty_modules[*]}]=$module_name
	fi
  fi  
done
if [ ${#empty_modules[*]} -gt 0 ]; then
  echo -e "<p>&nbsp;</p><p>READMEs were not available for these modules:</p><ul>\n" >> "$BASE_OUTPUT/index.html"
  for module_name in "${empty_modules[@]}"; do
    echo -e "<li><a href=\"http://cgit.freedesktop.org/libreoffice/core/tree/${module_name}\">${module_name}</a></li>\n" >> "$BASE_OUTPUT/index.html"
  done
  echo -e "</ul>\n" >> "$BASE_OUTPUT/index.html"
fi

footer "$BASE_OUTPUT/index.html"

## done
