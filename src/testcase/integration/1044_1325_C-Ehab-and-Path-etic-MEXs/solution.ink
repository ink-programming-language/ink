// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var q: dynamic = cpp_array(100005);

var f: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(100005);

var r: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      scanf("%d%d", (&f), (&t));
      q[f].push_back(i);
      q[t].push_back(i);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((q[i].size() >= 3))
      {
        r = 1;
        {
          var j: dynamic = 0;
          while ((j <= 2))
          {
            p[q[i][j]] = (j + 1);
            j += 1;
          }
        }
        break;
      }
      i += 1;
    }
  }
  if ((r == 0))
  {
    {
      var i: dynamic = 0;
      while ((i < (n - 1)))
      {
        write(i, "\n");
        i += 1;
      }
    }
    return 0;
  }
  var cnt: dynamic = 2;
  {
    var i: dynamic = 1;
    while ((i <= (n - 1)))
    {
      if ((p[i] == 3))
      {
        write(0, "\n");
      }
      if ((p[i] == 1))
      {
        write(1, "\n");
      }
      if ((p[i] == 2))
      {
        write(2, "\n");
      }
      if ((p[i] == 0))
      {
        write(cpp_update(cnt, "++"), "\n");
      }
      i += 1;
    }
  }
}
