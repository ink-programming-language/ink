// Translated from solution.cpp.

var graph: dynamic = cpp_array(300000);

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var l: dynamic = 0;
  var p: dynamic = 0;
  var w: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var flag: dynamic = 0;
  var k: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  var q: dynamic = 0;
  var r: dynamic = 0;
  var v: dynamic = cpp_uninitialized();
  read(n, m, v);
  t = (1 + ((((((n - 1)) * 1) * ((n - 2)))) / 2));
  if (((m > t) || (m < (n - 1))))
  {
    write(-1);
    return 0;
  } else
  {
    var z: dynamic = ((n + 1) - v);
    if ((z == v))
    {
      z = 1;
    }
    graph[z].push_back(v);
    m -= 1;
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        if ((i == z))
        {
          i += 1;
          continue;
        }
        {
          j = (i + 1);
          while ((j <= n))
          {
            if ((j == z))
            {
              j += 1;
              continue;
            }
            graph[i].push_back(j);
            m -= 1;
            if ((m == 0))
            {
              break;
            }
            j += 1;
          }
        }
        if ((m == 0))
        {
          break;
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        for (var it: dynamic in graph[i])
        {
          write(i, " ", it, "\n");
        }
        i += 1;
      }
    }
  }
  return 0;
}
