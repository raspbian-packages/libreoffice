validate_extensions() {
  INSTDIR=`mktemp -d`
  if HOME=$INSTDIR /usr/lib/libreoffice/program/unopkg list --bundled >/dev/null 2>/dev/null; then
	HOME=$INSTDIR /usr/lib/libreoffice/program/unopkg validate -v --bundled
  fi
}

