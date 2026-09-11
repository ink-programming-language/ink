// Translated from solution.cpp.

var N: dynamic = (1e6 + 10);

var a: dynamic = cpp_array(200);

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    memset(a, 0, cpp_sizeof((a)));
    var n: dynamic = cpp_uninitialized();
    var m: dynamic = cpp_uninitialized();
    read(n, m);
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        {
          var j: dynamic = 1;
          while ((j <= m))
          {
            var temp: dynamic = cpp_uninitialized();
            read(temp);
            a[((i + j) - 1)] ^= temp;
            j += 1;
          }
        }
        i += 1;
      }
    }
    var flag: dynamic = 1;
    {
      var i: dynamic = 1;
      while ((i <= ((n + m) - 1)))
      {
        if (a[i])
        {
          flag = 0;
        }
        i += 1;
      }
    }
    write(( (flag) ? "Jeel" : "Ashish"), "\n");
  }
}
