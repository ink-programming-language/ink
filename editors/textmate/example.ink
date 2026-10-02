// Open this file after importing Ink.tmbundle to inspect the active theme.
/* A block comment can contain "quotes", func, and 0xFF without changing scope. */

func 加法(Left: i32, Right: i32): i32
{
  return Left + Right;
}

func main(): i32
{
  var Decimal: i32 = 42;
  var Binary: i32 = 0b1010;
  var Octal: i32 = 0o17;
  var Hex: i32 = 0xFF;
  var Scientific = 1.25e-3;
  var Enabled: bool = true;
  var Letter = '中';
  var Escaped = "Hello\n\u4E2D\U0001F600";
  var Raw = r"C:\Ink\source\";
  var RawCharacter = r'\';
  var Multiline = """First line
if, // and /* here are string content.
Last line""";
  var RawMultiline = R"""C:\Ink
\n remains two characters.
""";
  var Continued = "First \
second";

  if (Enabled && Decimal > 0)
  {
    return 加法(Decimal, Binary);
  }
  else
  {
    return 0;
  }
}
