vs.1.1

# STATIC:  "TEX1"					"0..1"
# STATIC:  "TEX2"					"0..1"
# STATIC:  "TEX3"					"0..1"
# STATIC:  "BLUR"					"0..1"
# STATIC:  "VERTEXCOLOR"			"0..1"

# SKIP: $BLUR && ( $TEX1 || $TEX2 || $TEX3 )

#include "overlay_inc.vsh"

&Overlay( $BLUR, $TEX1, $TEX2, $TEX3, $VERTEXCOLOR );