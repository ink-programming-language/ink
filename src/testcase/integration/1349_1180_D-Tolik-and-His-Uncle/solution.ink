// Translated from solution.cpp.

var v: dynamic;

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var a = 0;
  var b = 0;
  var c: dynamic;
  var d: dynamic;
  var e: dynamic;
  var f = 0;
  var l: dynamic;
  var g: dynamic;
  var m: dynamic;
  var n: dynamic;
  var k: dynamic;
  var i: dynamic;
  var j: dynamic;
  var t: dynamic;
  var p: dynamic;
  var q: dynamic;
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
