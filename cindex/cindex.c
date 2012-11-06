#include <string.h>
#include "cindexInt.h"

/*
 * = = == === ===== ======== ============= =====================
 * == Copy of Tcl Core Code.
 */

/* Copy - Tcl Core, tclInt.h
 *----------------------------------------------------------------
 * Macro used by the Tcl core to clean out an object's internal
 * representation. Does not actually reset the rep's bytes. The ANSI C
 * "prototype" for this macro is:
 *
 * MODULE_SCOPE void	TclFreeIntRep(Tcl_Obj *objPtr);
 *----------------------------------------------------------------
 */

#define TclFreeIntRep(objPtr) \
    if ((objPtr)->typePtr != NULL && \
	    (objPtr)->typePtr->freeIntRepProc != NULL) { \
	(objPtr)->typePtr->freeIntRepProc(objPtr); \
    }

/*
 * The macro below is used to modify a "char" value (e.g. by casting it to an
 * unsigned character) so that it can be used safely with macros such as
 * isspace.
 */

#define UCHAR(c) ((unsigned char) (c))

static int		SetEndOffsetFromAny(Tcl_Interp* interp,
			    Tcl_Obj* objPtr);
static void		UpdateStringOfEndOffset(Tcl_Obj* objPtr);

/*
 * The following is the Tcl object type definition for an object that
 * represents a list index in the form, "end-offset". It is used as a
 * performance optimization in TclGetIntForIndex. The internal rep is an
 * integer, so no memory management is required for it.
 *
 * NOTE: To avoid clashing with the implementation of this type in the
 * Tcl Core this type has a different name.
 */

Tcl_ObjType tclEndOffsetType = {
    "c::index/end-offset",		/* name */
    NULL,				/* freeIntRepProc */
    NULL,				/* dupIntRepProc */
    UpdateStringOfEndOffset,		/* updateStringProc */
    SetEndOffsetFromAny
};

static int TclCheckBadOctal (Tcl_Interp *interp, CONST char *value);
static int TclFormatInt     (char *buffer, long n);

/*
 * The following table provides parsing information about each possible 8-bit
 * character. The table is designed to be referenced with either signed or
 * unsigned characters, so it has 384 entries. The first 128 entries
 * correspond to negative character values, the next 256 correspond to
 * positive character values. The last 128 entries are identical to the first
 * 128. The table is always indexed with a 128-byte offset (the 128th entry
 * corresponds to a character value of 0).
 *
 * The macro CHAR_TYPE is used to index into the table and return information
 * about its character argument. The following return values are defined.
 *
 * TYPE_NORMAL -	All characters that don't have special significance to
 *			the Tcl parser.
 * TYPE_SPACE -		The character is a whitespace character other than
 *			newline.
 * TYPE_COMMAND_END -	Character is newline or semicolon.
 * TYPE_SUBS -		Character begins a substitution or has other special
 *			meaning in ParseTokens: backslash, dollar sign, or
 *			open bracket.
 * TYPE_QUOTE -		Character is a double quote.
 * TYPE_CLOSE_PAREN -	Character is a right parenthesis.
 * TYPE_CLOSE_BRACK -	Character is a right square bracket.
 * TYPE_BRACE -		Character is a curly brace (either left or right).
 */

#define TYPE_NORMAL		0
#define TYPE_SPACE		0x1
#define TYPE_COMMAND_END	0x2
#define TYPE_SUBS		0x4
#define TYPE_QUOTE		0x8
#define TYPE_CLOSE_PAREN	0x10
#define TYPE_CLOSE_BRACK	0x20
#define TYPE_BRACE		0x40

#define CHAR_TYPE(c) (charTypeTable+128)[(int)(c)]

static const char charTypeTable[] = {
    /*
     * Negative character values, from -128 to -1:
     */

    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,

    /*
     * Positive character values, from 0-127:
     */

    TYPE_SUBS,        TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_SPACE,       TYPE_COMMAND_END, TYPE_SPACE,
    TYPE_SPACE,       TYPE_SPACE,       TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_SPACE,       TYPE_NORMAL,      TYPE_QUOTE,       TYPE_NORMAL,
    TYPE_SUBS,        TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_CLOSE_PAREN, TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_COMMAND_END,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_SUBS,
    TYPE_SUBS,        TYPE_CLOSE_BRACK, TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_BRACE,
    TYPE_NORMAL,      TYPE_BRACE,       TYPE_NORMAL,      TYPE_NORMAL,

    /*
     * Large unsigned character values, from 128-255:
     */

    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
    TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,      TYPE_NORMAL,
};

static int TclIsSpaceProc (char byte);
static int TclParseNumber (const char *bytes, int numBytes,
			   const char **endPtrPtr);

/*
 * = = == === ===== ======== ============= =====================
 */
/*
 *----------------------------------------------------------------------
 *
 * TclGetIntForIndex -- cindex_get --
 *
 *	This function returns an integer corresponding to the list index held
 *	in a Tcl object. The Tcl object's value is expected to be in the
 *	format integer([+-]integer)? or the format end([+-]integer)?.
 *
 * Results:
 *	The return value is normally TCL_OK, which means that the index was
 *	successfully stored into the location referenced by "indexPtr". If the
 *	Tcl object referenced by "objPtr" has the value "end", the value
 *	stored is "endValue". If "objPtr"s values is not of one of the
 *	expected formats, TCL_ERROR is returned and, if "interp" is non-NULL,
 *	an error message is left in the interpreter's result object.
 *
 * Side effects:
 *	The object referenced by "objPtr" might be converted to an integer,
 *	wide integer, or end-based-index object.
 *
 *----------------------------------------------------------------------
 */

int
cindex_get (
    Tcl_Interp *interp,		/* Interpreter to use for error reporting. If
				 * NULL, then no error message is left after
				 * errors. */
    Tcl_Obj *objPtr,		/* Points to an object containing either "end"
				 * or an integer. */
    int endValue,		/* The value to be stored at "indexPtr" if
				 * "objPtr" holds "end". */
    int *indexPtr)		/* Location filled in with an integer
				 * representing an index. */
{
    int length;
    char *opPtr, *bytes;

    if (Tcl_GetIntFromObj(NULL, objPtr, indexPtr) == TCL_OK) {
	return TCL_OK;
    }

    if (SetEndOffsetFromAny(NULL, objPtr) == TCL_OK) {
	/*
	 * If the object is already an offset from the end of the list, or can
	 * be converted to one, use it.
	 */

	*indexPtr = endValue + objPtr->internalRep.longValue;
	return TCL_OK;
    }

    bytes = Tcl_GetStringFromObj(objPtr, &length);

    /*
     * Leading whitespace is acceptable in an index.
     */

    while (length && TclIsSpaceProc(*bytes)) {
	bytes++;
	length--;
    }

    if (TclParseNumber(bytes, length, (const char **)&opPtr) == TCL_OK) {
	int code, first, second;
	char savedOp = *opPtr;

	if ((savedOp != '+') && (savedOp != '-')) {
	    goto parseError;
	}
	if (TclIsSpaceProc(opPtr[1])) {
	    goto parseError;
	}
	*opPtr = '\0';
	code = Tcl_GetInt(interp, bytes, &first);
	*opPtr = savedOp;
	if (code == TCL_ERROR) {
	    goto parseError;
	}
	if (TCL_ERROR == Tcl_GetInt(interp, opPtr+1, &second)) {
	    goto parseError;
	}
	if (savedOp == '+') {
	    *indexPtr = first + second;
	} else {
	    *indexPtr = first - second;
	}
	return TCL_OK;
    }

    /*
     * Report a parse error.
     */

  parseError:
    if (interp != NULL) {
	char *bytes = Tcl_GetString(objPtr);

	/*
	 * The result might not be empty; this resets it which should be both
	 * a cheap operation, and of little problem because this is an
	 * error-generation path anyway.
	 */

	Tcl_ResetResult(interp);
	Tcl_AppendResult(interp, "bad index \"", bytes,
		"\": must be integer?[+-]integer? or end?[+-]integer?", NULL);
	if (!strncmp(bytes, "end-", 4)) {
	    bytes += 4;
	}
	TclCheckBadOctal(interp, bytes);
    }

    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * UpdateStringOfEndOffset --
 *
 *	Update the string rep of a Tcl object holding an "end-offset"
 *	expression.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Stores a valid string in the object's string rep.
 *
 * This function does NOT free any earlier string rep. If it is called on an
 * object that already has a valid string rep, it will leak memory.
 *
 *----------------------------------------------------------------------
 */

static void
UpdateStringOfEndOffset(
    register Tcl_Obj* objPtr)
{
    char buffer[TCL_INTEGER_SPACE + sizeof("end") + 1];
    register int len;

    strcpy(buffer, "end");
    len = sizeof("end") - 1;
    if (objPtr->internalRep.longValue != 0) {
	buffer[len++] = '-';
	len += TclFormatInt(buffer+len, -(objPtr->internalRep.longValue));
    }
    objPtr->bytes = ckalloc((unsigned) len+1);
    memcpy(objPtr->bytes, buffer, (unsigned) len+1);
    objPtr->length = len;
}

/*
 *----------------------------------------------------------------------
 *
 * SetEndOffsetFromAny --
 *
 *	Look for a string of the form "end[+-]offset" and convert it to an
 *	internal representation holding the offset.
 *
 * Results:
 *	Returns TCL_OK if ok, TCL_ERROR if the string was badly formed.
 *
 * Side effects:
 *	If interp is not NULL, stores an error message in the interpreter
 *	result.
 *
 *----------------------------------------------------------------------
 */

static int
SetEndOffsetFromAny(
    Tcl_Interp *interp,		/* Tcl interpreter or NULL */
    Tcl_Obj *objPtr)		/* Pointer to the object to parse */
{
    int offset;			/* Offset in the "end-offset" expression */
    register char* bytes;	/* String rep of the object */
    int length;			/* Length of the object's string rep */

    /*
     * If it's already the right type, we're fine.
     */

    if (objPtr->typePtr == &tclEndOffsetType) {
	return TCL_OK;
    }

    /*
     * Check for a string rep of the right form.
     */

    bytes = Tcl_GetStringFromObj(objPtr, &length);
    if ((*bytes != 'e') || (strncmp(bytes, "end",
	    (size_t)((length > 3) ? 3 : length)) != 0)) {
	if (interp != NULL) {
	    Tcl_ResetResult(interp);
	    Tcl_AppendResult(interp, "bad index \"", bytes,
		    "\": must be end?[+-]integer?", NULL);
	}
	return TCL_ERROR;
    }

    /*
     * Convert the string rep.
     */

    if (length <= 3) {
	offset = 0;
    } else if ((length > 4) && ((bytes[3] == '-') || (bytes[3] == '+'))) {
	/*
	 * This is our limited string expression evaluator. Pass everything
	 * after "end-" to Tcl_GetInt, then reverse for offset.
	 */

	if (TclIsSpaceProc(bytes[4])) {
	    return TCL_ERROR;
	}
	if (Tcl_GetInt(interp, bytes+4, &offset) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (bytes[3] == '-') {
	    offset = -offset;
	}
    } else {
	/*
	 * Conversion failed. Report the error.
	 */

	if (interp != NULL) {
	    Tcl_ResetResult(interp);
	    Tcl_AppendResult(interp, "bad index \"", bytes,
		    "\": must be end?[+-]integer?", NULL);
	}
	return TCL_ERROR;
    }

    /*
     * The conversion succeeded. Free the old internal rep and set the new
     * one.
     */

    TclFreeIntRep(objPtr);
    objPtr->internalRep.longValue = offset;
    objPtr->typePtr = &tclEndOffsetType;

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * TclCheckBadOctal --
 *
 *	This function checks for a bad octal value and appends a meaningful
 *	error to the interp's result.
 *
 * Results:
 *	1 if the argument was a bad octal, else 0.
 *
 * Side effects:
 *	The interpreter's result is modified.
 *
 *----------------------------------------------------------------------
 */

static int
TclCheckBadOctal(
    Tcl_Interp *interp,		/* Interpreter to use for error reporting. If
				 * NULL, then no error message is left after
				 * errors. */
    CONST char *value)		/* String to check. */
{
    register CONST char *p = value;

    /*
     * A frequent mistake is invalid octal values due to an unwanted leading
     * zero. Try to generate a meaningful error message.
     */

    while (TclIsSpaceProc(*p)) {
	p++;
    }
    if (*p == '+' || *p == '-') {
	p++;
    }
    if (*p == '0') {
	if ((p[1] == 'o') || p[1] == 'O') {
	    p+=2;
	}
	while (isdigit(UCHAR(*p))) {	/* INTL: digit. */
	    p++;
	}
	while (TclIsSpaceProc(*p)) {
	    p++;
	}
	if (*p == '\0') {
	    /*
	     * Reached end of string.
	     */

	    if (interp != NULL) {
		/*
		 * Don't reset the result here because we want this result to
		 * be added to an existing error message as extra info.
		 */

		Tcl_AppendResult(interp, " (looks like invalid octal number)",
			NULL);
	    }
	    return 1;
	}
    }
    return 0;
}
/*
 *----------------------------------------------------------------------
 *
 * TclFormatInt --
 *
 *	This procedure formats an integer into a sequence of decimal digit
 *	characters in a buffer. If the integer is negative, a minus sign is
 *	inserted at the start of the buffer. A null character is inserted at
 *	the end of the formatted characters. It is the caller's
 *	responsibility to ensure that enough storage is available. This
 *	procedure has the effect of sprintf(buffer, "%ld", n) but is faster
 *	as proven in benchmarks.  This is key to UpdateStringOfInt, which
 *	is a common path for a lot of code (e.g. int-indexed arrays).
 *
 * Results:
 *	An integer representing the number of characters formatted, not
 *	including the terminating \0.
 *
 * Side effects:
 *	The formatted characters are written into the storage pointer to
 *	by the "buffer" argument.
 *
 *----------------------------------------------------------------------
 */

int
TclFormatInt(buffer, n)
    char *buffer;		/* Points to the storage into which the
				 * formatted characters are written. */
    long n;			/* The integer to format. */
{
    long intVal;
    int i;
    int numFormatted, j;
    char *digits = "0123456789";

    /*
     * Check first whether "n" is zero.
     */

    if (n == 0) {
	buffer[0] = '0';
	buffer[1] = 0;
	return 1;
    }

    /*
     * Check whether "n" is the maximum negative value. This is
     * -2^(m-1) for an m-bit word, and has no positive equivalent;
     * negating it produces the same value.
     */

    intVal = -n;			/* [Bug 3390638] Workaround for*/
    if (n == -n || intVal == n) {	/* broken compiler optimizers. */
	return sprintf(buffer, "%ld", n);
    }

    /*
     * Generate the characters of the result backwards in the buffer.
     */

    intVal = (n < 0? -n : n);
    i = 0;
    buffer[0] = '\0';
    do {
	i++;
	buffer[i] = digits[intVal % 10];
	intVal = intVal/10;
    } while (intVal > 0);
    if (n < 0) {
	i++;
	buffer[i] = '-';
    }
    numFormatted = i;

    /*
     * Now reverse the characters.
     */

    for (j = 0;  j < i;  j++, i--) {
	char tmp = buffer[i];
	buffer[i] = buffer[j];
	buffer[j] = tmp;
    }
    return numFormatted;
}
/*
 *----------------------------------------------------------------------
 *
 * TclIsSpaceProc --
 *
 *	Report whether byte is in the set of whitespace characters used by
 *	Tcl to separate words in scripts or elements in lists.
 *
 * Results:
 *	Returns 1, if byte is in the set, 0 otherwise.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
TclIsSpaceProc (char byte)
{
    return CHAR_TYPE(byte) & (TYPE_SPACE) || byte == '\n';
}

/*
 * = = == === ===== ======== ============= =====================
 */

/*
 *----------------------------------------------------------------------
 *
 * TclParseNumber --
 *
 *	Scans bytes, interpreted as characters in Tcl's internal encoding, and
 *	parses the longest prefix that is the string representation of a
 *	number in a format recognized by Tcl.
 *
 *	The arguments bytes, numBytes, and objPtr are the inputs which
 *	determine the string to be parsed. If bytes is non-NULL, it points to
 *	the first byte to be scanned. If bytes is NULL, then objPtr must be
 *	non-NULL, and the string representation of objPtr will be scanned
 *	(generated first, if necessary). The numBytes argument determines the
 *	number of bytes to be scanned. If numBytes is negative, the first NUL
 *	byte encountered will terminate the scan. If numBytes is non-negative,
 *	then no more than numBytes bytes will be scanned.
 *
 *	The argument flags is an input that controls the numeric formats
 *	recognized by the parser. The flag bits are:
 *
 *	- TCL_PARSE_INTEGER_ONLY:	accept only integer values; reject
 *		strings that denote floating point values (or accept only the
 *		leading portion of them that are integer values).
 *	- TCL_PARSE_SCAN_PREFIXES:	ignore the prefixes 0b and 0o that are
 *		not part of the [scan] command's vocabulary. Use only in
 *		combination with TCL_PARSE_INTEGER_ONLY.
 * 	- TCL_PARSE_OCTAL_ONLY:		parse only in the octal format, whether
 *		or not a prefix is present that would lead to octal parsing.
 *		Use only in combination with TCL_PARSE_INTEGER_ONLY.
 * 	- TCL_PARSE_HEXADECIMAL_ONLY:	parse only in the hexadecimal format,
 *		whether or not a prefix is present that would lead to
 *		hexadecimal parsing. Use only in combination with
 *		TCL_PARSE_INTEGER_ONLY.
 * 	- TCL_PARSE_DECIMAL_ONLY:	parse only in the decimal format, no
 *		matter whether a 0 prefix would normally force a different
 *		base.
 *	- TCL_PARSE_NO_WHITESPACE:	reject any leading/trailing whitespace
 *
 *	The arguments interp and expected are inputs that control error
 *	message generation. If interp is NULL, no error message will be
 *	generated. If interp is non-NULL, then expected must also be non-NULL.
 *	When TCL_ERROR is returned, an error message will be left in the
 *	result of interp, and the expected argument will appear in the error
 *	message as the thing TclParseNumber expected, but failed to find in
 *	the string.
 *
 *	The arguments objPtr and endPtrPtr as well as the return code are the
 *	outputs.
 *
 *	When the parser cannot find any prefix of the string that matches a
 *	format it is looking for, TCL_ERROR is returned and an error message
 *	may be generated and returned as described above. The contents of
 *	objPtr will not be changed. If endPtrPtr is non-NULL, a pointer to the
 *	character in the string that terminated the scan will be written to
 *	*endPtrPtr.
 *
 *	When the parser determines that the entire string matches a format it
 *	is looking for, TCL_OK is returned, and if objPtr is non-NULL, then
 *	the internal rep and Tcl_ObjType of objPtr are set to the "canonical"
 *	numeric value that matches the scanned string. If endPtrPtr is not
 *	NULL, a pointer to the end of the string will be written to *endPtrPtr
 *	(that is, either bytes+numBytes or a pointer to a terminating NUL
 *	byte).
 *
 *	When the parser determines that a partial string matches a format it
 *	is looking for, the value of endPtrPtr determines what happens:
 *
 *	- If endPtrPtr is NULL, then TCL_ERROR is returned, with error message
 *		generation as above.
 *
 *	- If endPtrPtr is non-NULL, then TCL_OK is returned and objPtr
 *		internals are set as above. Also, a pointer to the first
 *		character following the parsed numeric string is written to
 *		*endPtrPtr.
 *
 *	In some cases where the string being scanned is the string rep of
 *	objPtr, this routine can leave objPtr in an inconsistent state where
 *	its string rep and its internal rep do not agree. In these cases the
 *	internal rep will be in agreement with only some substring of the
 *	string rep. This might happen if the caller passes in a non-NULL bytes
 *	value that points somewhere into the string rep. It might happen if
 *	the caller passes in a numBytes value that limits the scan to only a
 *	prefix of the string rep. Or it might happen if a non-NULL value of
 *	endPtrPtr permits a TCL_OK return from only a partial string match. It
 *	is the responsibility of the caller to detect and correct such
 *	inconsistencies when they can and do arise.
 *
 * Results:
 *	Returns a standard Tcl result.
 *
 * Side effects:
 *	The string representaton of objPtr may be generated.
 *
 *	The internal representation and Tcl_ObjType of objPtr may be changed.
 *	This may involve allocation and/or freeing of memory.
 *
 *----------------------------------------------------------------------
 */

/* NOTE: Reduced, Fixed Arguments removed, and code modified per their values
 */
static int
TclParseNumber(
    const char *bytes,		/* Pointer to the start of the string to
				 * scan. */
    int numBytes,		/* Maximum number of bytes to scan, see
				 * above. */
    const char **endPtrPtr)	/* Place to store pointer to the character
				 * that terminated the scan. */
{
    enum State {
	INITIAL, SIGNUM, ZERO, ZERO_X,
	ZERO_O, ZERO_B, BINARY,
	HEXADECIMAL, OCTAL, BAD_OCTAL, DECIMAL,
    } state = INITIAL;
    enum State acceptState = INITIAL;

    int signum = 0;		/* Sign of the number being parsed */
    Tcl_WideUInt significandWide = 0;
				/* Significand of the number being parsed (if
				 * no overflow) */
    int numSigDigs = 0;		/* Number of significant digits in the decimal
				 * significand */
    int numTrailZeros = 0;	/* Number of trailing zeroes at the current
				 * point in the parse. */
    const char *p;		/* Pointer to next character to scan */
    size_t len;			/* Number of characters remaining after p */
    const char *acceptPoint;	/* Pointer to position after last character in
				 * an acceptable number */
    size_t acceptLen;		/* Number of characters following that
				 * point. */
    int status = TCL_OK;	/* Status to return to caller */
    char d = 0;			/* Last hexadecimal digit scanned; initialized
				 * to avoid a compiler warning. */
    int shift = 0;		/* Amount to shift when accumulating binary */
    int explicitOctal = 0;

#define ALL_BITS	(~(Tcl_WideUInt)0)
#define MOST_BITS	(ALL_BITS >> 1)

    p = bytes;
    len = numBytes;
    acceptPoint = p;
    acceptLen = len;
    while (1) {
	char c = len ? *p : '\0';
	switch (state) {

	case INITIAL:
	    /*
	     * Initial state. Acceptable characters are +, -, digits, period,
	     * I, N, and whitespace.
	     */

	    if (TclIsSpaceProc(c)) {
	      goto endgame;
	    } else if (c == '+') {
		state = SIGNUM;
		break;
	    } else if (c == '-') {
		signum = 1;
		state = SIGNUM;
		break;
	    }
	    /* FALLTHROUGH */

	case SIGNUM:
	    /*
	     * Scanned a leading + or -. Acceptable characters are digits,
	     * period, I, and N.
	     */

	    if (c == '0') {
	        state = ZERO;
		break;
	    } else if (isdigit(UCHAR(c))) {
		significandWide = c - '0';
		numSigDigs = 1;
		state = DECIMAL;
		break;
	    }
	    goto endgame;

	case ZERO:
	    /*
	     * Scanned a leading zero (perhaps with a + or -). Acceptable
	     * inputs are digits, period, X, b, and E. If 8 or 9 is encountered,
	     * the number can't be octal. This state and the OCTAL state
	     * differ only in whether they recognize 'X' and 'b'.
	     */

	    acceptState = state;
	    acceptPoint = p;
	    acceptLen = len;
	    if (c == 'x' || c == 'X') {
		state = ZERO_X;
		break;
	    }
	    if (c == 'b' || c == 'B') {
		state = ZERO_B;
		break;
	    }
	    if (c == 'o' || c == 'O') {
		explicitOctal = 1;
		state = ZERO_O;
		break;
	    }
#ifdef KILL_OCTAL
	    goto decimal;
#endif
	    /* FALLTHROUGH */

	case OCTAL:
	    /*
	     * Scanned an optional + or -, followed by a string of octal
	     * digits. Acceptable inputs are more digits, period, or E. If 8
	     * or 9 is encountered, commit to floating point.
	     */

	    acceptState = state;
	    acceptPoint = p;
	    acceptLen = len;
	    /* FALLTHROUGH */
	case ZERO_O:
	zeroo:
	    if (c == '0') {
		numTrailZeros++;
		state = OCTAL;
		break;
	    } else if (c >= '1' && c <= '7') {
		if (numSigDigs != 0) {
		    numSigDigs += numTrailZeros+1;
		} else {
		    numSigDigs = 1;
		}
		numTrailZeros = 0;
		state = OCTAL;
		break;
	    }
	    /* FALLTHROUGH */

	case BAD_OCTAL:
	    goto endgame;

	    /*
	     * Scanned 0x. If state is HEXADECIMAL, scanned at least one
	     * character following the 0x. The only acceptable inputs are
	     * hexadecimal digits.
	     */

	case HEXADECIMAL:
	    acceptState = state;
	    acceptPoint = p;
	    acceptLen = len;
	    /* FALLTHROUGH */

	case ZERO_X:
	zerox:
	    if (c == '0') {
		numTrailZeros++;
		state = HEXADECIMAL;
		break;
	    } else if (isdigit(UCHAR(c))) {
		d = (c-'0');
	    } else if (c >= 'A' && c <= 'F') {
		d = (c-'A'+10);
	    } else if (c >= 'a' && c <= 'f') {
		d = (c-'a'+10);
	    } else {
		goto endgame;
	    }
	    numTrailZeros = 0;
	    state = HEXADECIMAL;
	    break;

	case BINARY:
	    acceptState = state;
	    acceptPoint = p;
	    acceptLen = len;
	case ZERO_B:
	    if (c == '0') {
		numTrailZeros++;
		state = BINARY;
		break;
	    } else if (c != '1') {
		goto endgame;
	    }
	    numTrailZeros = 0;
	    state = BINARY;
	    break;

	case DECIMAL:
	    /*
	     * Scanned an optional + or - followed by a string of decimal
	     * digits.
	     */

#ifdef KILL_OCTAL
	decimal:
#endif
	    acceptState = state;
	    acceptPoint = p;
	    acceptLen = len;
	    if (c == '0') {
		numTrailZeros++;
		state = DECIMAL;
		break;
	    } else if (isdigit(UCHAR(c))) {
		numSigDigs += numTrailZeros+1;
		numTrailZeros = 0;
		state = DECIMAL;
		break;
	    }
	    goto endgame;
	}
	p++;
	len--;
    }

  endgame:
    if (acceptState == INITIAL) {
	/*
	 * No numeric string at all found.
	 */

	status = TCL_ERROR;
	if (endPtrPtr != NULL) {
	    *endPtrPtr = p;
	}
    } else {
	/*
	 * Back up to the last accepting state in the lexer.
	 */

	p = acceptPoint;
	len = acceptLen;
	if (endPtrPtr == NULL) {
	    if ((len != 0) && ((numBytes > 0) || (*p != '\0'))) {
		status = TCL_ERROR;
	    }
	} else {
	    *endPtrPtr = p;
	}
    }

    return status;
}

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
