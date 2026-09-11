// Translated from solution.cpp.

var i: dynamic = 0;

var j: dynamic = 0;

var k: dynamic = 0;

var arr: dynamic = cpp_array(1001, 101);

func solve() -> dynamic
{
  memset(arr, 0, cpp_sizeof((arr)));
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var o: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  read(n, m, o, t);
  {
    var i: dynamic = 0;
    while ((i <= (n - 1)))
    {
      {
        var j: dynamic = 0;
        while ((j <= (m - 1)))
        {
          read(arr[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var v: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((i < n))
    {
      var s: dynamic = "";
      {
        j = 0;
        while ((j < m))
        {
          if ((int_cpp(arr[i][j]) == 46))
          {
            s += arr[i][j];
          } else
          {
            v.push_back(s);
            s = "";
          }
          j += 1;
        }
      }
      v.push_back(s);
      i += 1;
    }
  }
  if (((2 * o) <= t))
  {
    var tt: dynamic = 0;
    for (var s: dynamic in v)
    {
      tt += cpp_cast((s).size());
    }
    write((tt * o), "\n");
  } else
  {
    var oo: dynamic = 0;
    var tt: dynamic = 0;
    for (var s: dynamic in v)
    {
      var ss: dynamic = cpp_cast((s).size());
      if ((ss & 1))
      {
        oo += 1;
        tt += (ss / 2);
      } else
      {
        tt += (ss / 2);
      }
    }
    write(((oo * o) + (tt * t)), "\n");
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var t: dynamic = 1;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
}
