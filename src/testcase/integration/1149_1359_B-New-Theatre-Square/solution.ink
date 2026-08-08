// Translated from solution.cpp.

var i = 0;

var j = 0;

var k = 0;

var arr = cpp_array(1001, 101);

func solve()
{
  memset(arr, 0, cpp_sizeof((arr)));
  var n: dynamic;
  var m: dynamic;
  var o: dynamic;
  var t: dynamic;
  read(n, m, o, t);
  {
    var i = 0;
    while ((i <= (n - 1)))
    {
      {
        var j = 0;
        while ((j <= (m - 1)))
        {
          read(arr[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var v: dynamic;
  {
    i = 0;
    while ((i < n))
    {
      var s = "";
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
    var tt = 0;
    for (var s in v)
    {
      tt += cpp_cast((s).size());
    }
    write((tt * o), "\n");
  } else
  {
    var oo = 0;
    var tt = 0;
    for (var s in v)
    {
      var ss = cpp_cast((s).size());
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

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var t = 1;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
}
