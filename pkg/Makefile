PORTNAME=       bsdbook
PORTVERSION=    1.6
CATEGORIES=     devel
DISTNAME=       ${PORTNAME}-${PORTVERSION}

USE_GITHUB=     yes
GH_ACCOUNT=     TwelveFacedJanus
GH_PROJECT=     BSDBook
GH_TAGNAME=     v${PORTVERSION}

MAINTAINER=     twofaced-janus@yandex.ru
COMMENT=        Simple notes manager for bsd.
LICENSE=        MIT

USES=           gmake ncurses
CFLAGS+=        -I${LOCALBASE}/include
LDFLAGS=       -L${LOCALBASE}/lib -lncurses

# Installation target
.PHONY: do-install
do-install:
	@${MKDIR} ${STAGEDIR}${PREFIX}/bin
	@${INSTALL} -m 555 ${WRKSRC}/bsdnotes ${STAGEDIR}${PREFIX}/bin/bsdnotes
	@${MKDIR} ${STAGEDIR}${PREFIX}/share/doc/${PORTNAME}
	@${INSTALL} -m 644 ${WRKSRC}/README.md ${STAGEDIR}${PREFIX}/share/doc/${PORTNAME}/README.md

.include <bsd.port.mk>

