// Translated from solution.cpp.

func read() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 1;
  var c: dynamic = getchar();
  {
    while (((c < cpp_char("0")) || (c > cpp_char("9"))))
    {
      if ((c == cpp_char("-")))
      {
        f = -1;
      }
      c = getchar();
    }
  }
  {
    while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
    {
      x = (((x * 10) + c) - 48);
      c = getchar();
    }
  }
  return (x * f);
}

func write(x: dynamic) -> dynamic
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    x = (-x);
  }
  if ((x >= 10))
  {
    write((x / 10));
  }
  putchar((((x % 10)) + cpp_char("0")));
}

func writeln(x: dynamic) -> dynamic
{
  write(x);
  puts("");
}

var oo: dynamic = 0x3f3f3f3f;

var inf: dynamic = oo;

var f: dynamic = cpp_array(26);

var ans: dynamic = cpp_uninitialized();

var s: dynamic = cpp_array(100005);

var ch: dynamic = cpp_array(100005);

var n: dynamic = cpp_uninitialized();

var Q: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%s", (s + 1));
  n = strlen((s + 1));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      s[i] -= cpp_char("a");
      f[s[i]][i] = 1;
      i += 1;
    }
  }
  Q = read();
  while (cpp_update(Q, "--"))
  {
    var op: dynamic = read();
    if ((op == 1))
    {
      var pos: dynamic = read();
      var ch: dynamic = getchar();
      f[s[pos]][pos] = 0;
      s[pos] = (ch - cpp_char("a"));
      f[s[pos]][pos] = 1;
    } else
    {
      var l: dynamic = read();
      var r: dynamic = read();
      scanf("%s", ch);
      var len: dynamic = strlen(ch);
      if ((((r - l) + 1) < len))
      {
        puts("0");
        continue;
      }
      ans.set();
      ans <<= ((l - 1));
      ans ^= ((ans << ((((r - l) + 2) - len))));
      {
        var i: dynamic = 0;
        while ((i < len))
        {
          ans = (((ans << 1)) & f[(ch[i] - cpp_char("a"))]);
          i += 1;
        }
      }
      writeln(ans.count());
    }
  }
  return 0;
}
