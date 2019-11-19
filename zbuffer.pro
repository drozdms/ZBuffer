CONFIG       += ordered
TEMPLATE      = subdirs
SUBDIRS       = ZBuffer

QT_VERSION=$$[QT_VERSION]

contains( QT_VERSION, "^5.*" ) {
	cache()
}
