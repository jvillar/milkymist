#!/usr/bin/env python

# NOR StrataFlash Flasher for Spartan 3E Starter Kit from Digilent
# base on Very simple serial terminal from Chris Liechti
#
# (C)2002-2009 Chris Liechti <cliechti@gmx.net>
# (C)2009 Jose Ignacio Villar <jose@dte.us.es>

# Input characters are sent directly (only LF -> CR/LF/CRLF translation is
# done), received characters are displayed as is (or escaped trough pythons
# repr, useful for debug purposes)


import sys, os, serial, threading

EXITCHARCTER = '\x1d'   # GS/CTRL+]
MENUCHARACTER = '\x14'  # Menu: CTRL+T


def key_description(character):
    """generate a readable description for a key"""
    ascii_code = ord(character)
    if ascii_code < 32:
        return 'Ctrl+%c' % (ord('@') + ascii_code)
    else:
        return repr(character)

# help text, starts with blank line! it's a function so that the current values
# for the shortcut keys is used and not the value at program start
def get_help_text():
    return """
--- pySerial - miniterm - help
---
--- %(exit)-8s Exit program
--- %(menu)-8s Menu escape key, followed by:
--- Menu keys:
---       %(itself)-8s Send the menu character itself to remote
---       %(exchar)-8s Send the exit character to remote
---       %(info)-8s Show info
---       %(upload)-8s Upload file (prompt will be shown)
--- Toggles:
---       %(rts)s  RTS          %(echo)s  local echo
---       %(dtr)s  DTR          %(break)s  BREAK
---       %(lfm)s  line feed    %(repr)s  Cycle repr mode
---
--- Port settings (%(menu)s followed by the following):
--- 7 8           set data bits
--- n e o s m     change parity (None, Even, Odd, Space, Mark)
--- 1 2 3         set stop bits (1, 2, 1.5)
--- b             change baud rate
--- x X           disable/enable software flow control
--- r R           disable/enable hardware flow control
""" % {
    'exit': key_description(EXITCHARCTER),
    'menu': key_description(MENUCHARACTER),
    'rts': key_description('\x12'),
    'repr': key_description('\x01'),
    'dtr': key_description('\x04'),
    'lfm': key_description('\x0c'),
    'break': key_description('\x02'),
    'echo': key_description('\x05'),
    'info': key_description('\x09'),
    'upload': key_description('\x15'),
    'itself': key_description(MENUCHARACTER),
    'exchar': key_description(EXITCHARCTER),
}


CONVERT_CRLF = 2
CONVERT_CR   = 1
CONVERT_LF   = 0
NEWLINE_CONVERISON_MAP = ('\n', '\r', '\r\n')
LF_MODES = ('LF', 'CR', 'CR/LF')

REPR_MODES = ('raw', 'some control', 'all control', 'hex')

class Miniterm:
    def __init__(self, port, baudrate, parity, rtscts, xonxoff, echo=False, convert_outgoing=CONVERT_CRLF, repr_mode=0):
        try:
            self.serial = serial.serial_for_url(port, baudrate, parity=parity, rtscts=rtscts, xonxoff=xonxoff, timeout=1)
        except AttributeError:
            # happens when the installed pyserial is older than 2.5. use the
            # Serial class directly then.
            self.serial = serial.Serial(port, baudrate, parity=parity, rtscts=rtscts, xonxoff=True, timeout=1)
        self.echo = echo
        self.repr_mode = repr_mode
        self.convert_outgoing = convert_outgoing
        self.newline = NEWLINE_CONVERISON_MAP[self.convert_outgoing]
        self.dtr_state = True
        self.rts_state = True
        self.break_state = False

    def wait_ready(self):
        while True:
            self.serial.flush()
            data = self.serial.read(1)
            if data == '\r' and self.convert_outgoing == CONVERT_CR:
                sys.stdout.write('\n')
            else:
                sys.stdout.write(data)

            if data.count(">") > 0:
                sys.stdout.write("\n--- READY DETECTED ---\n")
                break
            sys.stdout.flush()

    def sendcommand(self, command):
#        sys.stdout.write('--- ISSUED COMMAND: %s: %s ---\n' % (command))
#        sys.stdout.write('--- WAITING TO FINISH COMMAND: %s: %s ---\n' % (command))

        if command[0] == "erase":
            self.serial.write("e")
            self.serial.write("Y")
        elif command[0] == "status":
            self.serial.write("s")
        elif command[0] == "id":
            self.serial.write("i")
        elif command[0] == "read":
            self.serial.write("r")
            self.serial.write(command[1])
        elif command[0] == "program":
            self.serial.write("p")
            filename = command[1]
            try:
                file = open(filename, 'r')
                sys.stdout.write('--- Sending file %s ---\n' % filename)
                lines = file.readlines()
                for line in lines:
                    line = line.rstrip('\r\n')
                    self.serial.write(line)
                    self.serial.write('\r')
                    # Wait for output buffer to drain.
                    self.serial.flush()
                sys.stdout.write('\n--- File %s sent ---\n' % filename)
            except IOError, e:
#                sys.stderr.write('--- ERROR opening file %s: %s ---\n' % (filename, e))
                self.serial.write(":00000001FF\n")
                self.serial.flush()

	self.wait_ready()

        sys.stdout.write('--- FINISHED COMMAND: %s: %s ---\n' % (command))
        sys.stdout.flush()


def main():
    import optparse

    parser = optparse.OptionParser(
        usage = "%prog [options] [port [baudrate]] [--erase] [--program FILE]",
        description = "NOR FLASHER - A serial port based NOR memory flasher for S3E Starter Kit."
    )

    parser.add_option("-p", "--port",
        dest = "port",
        help = "port, a number (default 0) or a device name (deprecated option)",
        default = None
    )

    parser.add_option("-b", "--baud",
        dest = "baudrate",
        action = "store",
        type = 'int',
        help = "set baud rate, default %default",
        default = 115200
    )

    parser.add_option("--parity",
        dest = "parity",
        action = "store",
        help = "set parity, one of [N, E, O, S, M], default=N",
        default = 'N'
    )

    parser.add_option("-e", "--echo",
        dest = "echo",
        action = "store_true",
        help = "enable local echo (default off), default %default",
        default = False
    )

    parser.add_option("--rtscts",
        dest = "rtscts",
        action = "store_true",
        help = "enable RTS/CTS flow control (default off), default %default",
        default = False
    )

    parser.add_option("--xonxoff",
        dest = "xonxoff",
        action = "store_true",
        help = "enable software flow control (default off), default %default",
        default = True
    )

    parser.add_option("--cr",
        dest = "cr",
        action = "store_true",
        help = "do not send CR+LF, send CR only, default %default",
        default = True
    )

    parser.add_option("--lf",
        dest = "lf",
        action = "store_true",
        help = "do not send CR+LF, send LF only, default %default",
        default = False
    )

    parser.add_option("-D", "--debug",
        dest = "repr_mode",
        action = "count",
        help = """debug received data (escape non-printable chars)
--debug can be given multiple times:
0: just print what is received
1: escape non-printable characters, do newlines as unusual
2: escape non-printable characters, newlines too
3: hex dump everything""",
        default = 0
    )

    parser.add_option("--rts",
        dest = "rts_state",
        action = "store",
        type = 'int',
        help = "set initial RTS line state (possible values: 0, 1)",
        default = None
    )

    parser.add_option("--dtr",
        dest = "dtr_state",
        action = "store",
        type = 'int',
        help = "set initial DTR line state (possible values: 0, 1)",
        default = None
    )

    parser.add_option("-q", "--quiet",
        dest = "quiet",
        action = "store_true",
        help = "suppress non error messages",
        default = False
    )

    parser.add_option("--exit-char",
        dest = "exit_char",
        action = "store",
        type = 'int',
        help = "ASCII code of special character that is used to exit the application",
        default = 0x1d
    )

    parser.add_option("--erase",
        dest = "erase",
        action = "store_true",
        help = "deletes memory contents before any write operation, default %default",
        default = False
    )

    parser.add_option("--program",
        dest = "program",
        action = "store",
        help = "file to program to the flash memory, default %default",
        default = None
    )

    (options, args) = parser.parse_args()

    options.parity = options.parity.upper()
    if options.parity not in 'NEOSM':
        parser.error("invalid parity")

    if options.cr and options.lf:
        parser.error("only one of --cr or --lf can be specified")

    if options.program:
        try:
            file = open(options.program, 'r')
            file.close()
        except IOError:
            parser.error("Supplied file does not exists or is not readeable")

    global EXITCHARCTER
    EXITCHARCTER = chr(options.exit_char)

    port = options.port
    baudrate = options.baudrate
    if args:
        if options.port is not None:
            parser.error("no arguments are allowed, options only when --port is given")
        port = args.pop(0)
        if args:
            try:
                baudrate = int(args[0])
            except ValueError:
                parser.error("baud rate must be a number, not %r" % args[0])
            args.pop(0)
        if args:
            parser.error("too many arguments")
    else:
        if port is None: port = 0

    convert_outgoing = CONVERT_CRLF
    if options.cr:
        convert_outgoing = CONVERT_CR
    elif options.lf:
        convert_outgoing = CONVERT_LF

    try:
        miniterm = Miniterm(
            port,
            baudrate,
            options.parity,
            rtscts=options.rtscts,
            xonxoff=options.xonxoff,
            echo=options.echo,
            convert_outgoing=convert_outgoing,
            repr_mode=options.repr_mode,
        )
    except serial.SerialException, e:
        sys.stderr.write("could not open port %r: %s\n" % (port, e))
        sys.exit(1)

    if not options.quiet:
        sys.stderr.write('--- NOR FLASHER on %s: %d,%s,%s,%s ---\n' % (
            miniterm.serial.portstr,
            miniterm.serial.baudrate,
            miniterm.serial.bytesize,
            miniterm.serial.parity,
            miniterm.serial.stopbits,
        ))


    if options.dtr_state is not None:
        if not options.quiet:
            sys.stderr.write('--- forcing DTR %s\n' % (options.dtr_state and 'active' or 'inactive'))
        miniterm.serial.setDTR(options.dtr_state)
        miniterm.dtr_state = options.dtr_state
    if options.rts_state is not None:
        if not options.quiet:
            sys.stderr.write('--- forcing RTS %s\n' % (options.rts_state and 'active' or 'inactive'))
        miniterm.serial.setRTS(options.rts_state)
        miniterm.rts_state = options.rts_state

    miniterm.wait_ready()

    if options.erase:
            miniterm.sendcommand(("erase", None))
	    if options.program:
                miniterm.sendcommand(("read", "000000"))

    if options.program:
        miniterm.sendcommand(("program", options.program))

    miniterm.sendcommand(("read", "000000"))
    miniterm.sendcommand(("status", None))

    if not options.quiet:
        sys.stderr.write("\n--- exit ---\n")


if __name__ == '__main__':
    main()
