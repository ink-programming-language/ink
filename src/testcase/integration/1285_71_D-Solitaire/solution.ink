// Translated from solution.cpp.

var firsts: dynamic = "23456789TJQKA";

var seconds: dynamic = "CDHS";

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(55, 55);

var cur: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var wh: dynamic = cpp_uninitialized();

func go(x: dynamic, y: dynamic) -> dynamic
{
  if ((y == m))
  {
    y = 0;
    x += 1;
  }
  if ((x == n))
  {
    var goods: dynamic = cpp_uninitialized();
    {
      var i1: dynamic = 0;
      while (((i1 + 2) < n))
      {
        {
          var j1: dynamic = 0;
          while (((j1 + 2) < m))
          {
            var f: dynamic = cpp_uninitialized();
            var s: dynamic = cpp_uninitialized();
            {
              var x: dynamic = 0;
              while ((x < 3))
              {
                {
                  var y: dynamic = 0;
                  while ((y < 3))
                  {
                    f.insert(a[(i1 + x)][(j1 + y)][0]);
                    s.insert(a[(i1 + x)][(j1 + y)][1]);
                    y += 1;
                  }
                }
                x += 1;
              }
            }
            if (((f.size() == 9) || (s.size() == 1)))
            {
              goods.emplace_back(i1, j1);
            }
            j1 += 1;
          }
        }
        i1 += 1;
      }
    }
    for (var c1: dynamic in goods)
    {
      for (var c2: dynamic in goods)
      {
        if (((abs((c1.first - c2.first)) >= 3) || (abs((c1.second - c2.second)) >= 3)))
        {
          ans = cur;
          wh = [c1, c2];
          return true;
        }
      }
    }
    return false;
  }
  var th: dynamic = a[x][y];
  if (((th != "J1") && (th != "J2")))
  {
    return go(x, (y + 1));
  }
  for (var f: dynamic in firsts)
  {
    for (var s: dynamic in seconds)
    {
      var rep: dynamic = [f, s];
      var ok: dynamic = true;
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          {
            var j: dynamic = 0;
            while ((j < m))
            {
              ok &= (a[i][j] != rep);
              j += 1;
            }
          }
          i += 1;
        }
      }
      if ((!ok))
      {
        continue;
      }
      a[x][y] = rep;
      cur.emplace_back(th, rep);
      if (go(x, (y + 1)))
      {
        return true;
      }
      cur.pop_back();
      a[x][y] = th;
    }
  }
  return false;
}

func main() -> dynamic
{
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          read(a[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  if (go(0, 0))
  {
    write("Solution exists.\n");
    if (ans.empty())
    {
      write("There are no jokers.\n");
    } else if ((ans.size() == 1))
    {
      write("Replace ", ans[0].first, " with ", ans[0].second, ".\n");
    } else
    {
      var x: dynamic = ans[0].second;
      var y: dynamic = ans[1].second;
      if ((ans[0].first != "J1"))
      {
        swap(x, y);
      }
      write("Replace J1 with ", x, " and J2 with ", y, ".\n");
    }
    write("Put the first square to (", (wh[0].first + 1), ", ", (wh[0].second + 1), ").\n");
    write("Put the second square to (", (wh[1].first + 1), ", ", (wh[1].second + 1), ").\n");
  } else
  {
    write("No solution.\n");
  }
}
