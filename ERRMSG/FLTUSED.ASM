;-----------------------------------------------------------------------------
; FLTUSED.ASM -- Object module used to force MSC 5.1 to include floating
;                point support. This module is included to allow the
;                vsprintf function to be able to handle floating point
;                numbers if necessary.
;
; Modifications --
;   28-Nov-1989 : Initial version.
;-----------------------------------------------------------------------------

EXTRN   __fltused:NEAR
END
