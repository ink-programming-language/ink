// Translated from solution.cpp.

var N = (1e6 + 10);

var a = cpp_array(200);

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    memset(a, 0, cpp_sizeof((a)));
    var n: dynamic;
    var m: dynamic;
    read(n, m);
    {
      var i = 1;
      while ((i <= n))
      {
        {
          var j = 1;
          while ((j <= m))
          {
            var temp: dynamic;
            read(temp);
            a[((i + j) - 1)] ^= temp;
            j += 1;
          }
        }
        i += 1;
      }
    }
    var flag = 1;
    {
      var i = 1;
      while ((i <= ((n + m) - 1)))
      {
        if (a[i])
        {
          flag = 0;
        }
        i += 1;
      }
    }
    write((if (flag) "Jeel" else "Ashish"), "\n");
  }
}
