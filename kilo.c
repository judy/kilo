/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** defines ***/

#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/

struct termios orig_termios;

/*** terminal ***/

void die(const char *s) {
  perror(s); // perror() looks at the global errno variable and prints a descriptive error message for it
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
    die("tcsetattr");
  }
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
  atexit(disableRawMode);

  struct termios raw = orig_termios;
  // IXON = Turn off Ctrl+S and Ctrl+Q signals (flow control)
  // ICRNL = Turn Ctrl+M back into its own byte, stop letting term turn carriage returns into new lines
  // The rest are .:legacy:. raw mode additions.
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  // OPOST = Stop output processing (we now have to write \r\n ourselves)
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8); // .:legacy:. raw mode additions.
  // ECHO = don't echo every typed character
  // ICANON = one character at a time, don't wait for user to hit Enter
  // ISIG = Turn off Ctrl+V signals
  // ISIG = Turn off Ctrl+C and Ctrl+Z signals
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  // VMIN = sets the minimum number of bytes of input needed before read() can return.
  // VTIME = sets the maximum amount of time to wait before read() returns.
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

char editorReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  return c;
}

/*** input ***/

void editorProcessKeypress() {
  char c = editorReadKey();

  switch (c) {
    case CTRL_KEY('q'):
      exit(0);
      break;
  }
}

/*** init ***/

int main() {
  enableRawMode();

  while (1) {
    editorProcessKeypress();
  }

  return 0;
}
