meta:
	ADDON_NAME = ofxNNG
	ADDON_DESCRIPTION = nanomsg-next-geeration Wrapper for openFrameworks. https://nng.nanomsg.org/
	ADDON_AUTHOR = Nariaki Iwatani
	ADDON_TAGS = "NNG" "Network" "Messaging"
	ADDON_URL = https://github.com/nariakiiwatani/ofxNNG

vs:
	ADDON_DLLS_TO_COPY += windows-copy-to-bin/nng.x86.dll
	ADDON_DLLS_TO_COPY += windows-copy-to-bin/nng.x64.dll

