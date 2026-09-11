// Translated from solution.cpp.

var t: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var num: dynamic = cpp_array(30);

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n);
    memset(num, 0, cpp_sizeof((num)));
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        read(x);
        var cnt: dynamic = 0;
        while (x)
        {
          if ((x & 1))
          {
            num[cnt] += 1;
          }
          x >>= 1;
          cnt += 1;
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 29;
      while ((i >= 0))
      {
        if ((num[i] & 1))
        {
          if (((!((((n - num[i])) & 1))) && (((num[i] % 4) == 3))))
          {
            write("LOSE", cpp_char("\n"));
          } else
          {
            write("WIN", cpp_char("\n"));
          }
          cpp_goto("goto end;");
        }
        i -= 1;
      }
    }
    write("DRAW", cpp_char("\n"));
  }
  return 0;
}
