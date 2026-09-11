// Translated from solution.cpp.

var v: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var a: dynamic = 0;
  var b: dynamic = 0;
  var c: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  var e: dynamic = cpp_uninitialized();
  var f: dynamic = 0;
  var l: dynamic = cpp_uninitialized();
  var g: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  read(m, n);
  {
    i = 1;
    while ((i <= (n / 2)))
    {
      {
        j = 1;
        while ((j <= m))
        {
          write(j, cpp_char(" "), i, cpp_char("\n"));
          write(((m - j) + 1), cpp_char(" "), ((n - i) + 1), cpp_char("\n"));
          j += 1;
        }
      }
      i += 1;
    }
  }
  if ((n % 2))
  {
    d = ((n / 2) + 1);
    {
      i = 1;
      while ((i <= (m / 2)))
      {
        write(i, cpp_char(" "), d, cpp_char("\n"));
        write(((m - i) + 1), cpp_char(" "), d, cpp_char("\n"));
        i += 1;
      }
    }
    if ((m % 2))
    {
      write(((m / 2) + 1), cpp_char(" "), d, cpp_char("\n"));
    }
  }
  return 0;
}
