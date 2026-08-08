// Translated from solution.cpp.

var graph = cpp_array(300000);

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic;
  var m: dynamic;
  var j: dynamic;
  var l = 0;
  var p = 0;
  var w: dynamic;
  var i: dynamic;
  var flag = 0;
  var k: dynamic;
  var t: dynamic;
  var d: dynamic;
  var q = 0;
  var r = 0;
  var v: dynamic;
  read(n, m, v);
  t = (1 + ((((((n - 1)) * 1) * ((n - 2)))) / 2));
  if (((m > t) || (m < (n - 1))))
  {
    write(-1);
    return 0;
  } else
  {
    var z = ((n + 1) - v);
    if ((z == v))
    {
      z = 1;
    }
    graph[z].push_back(v);
    m -= 1;
    {
      var i = 1;
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
      var i = 1;
      while ((i <= n))
      {
        for (var it in graph[i])
        {
          write(i, " ", it, "\n");
        }
        i += 1;
      }
    }
  }
  return 0;
}
