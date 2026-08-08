// Translated from solution.cpp.

var firsts = "23456789TJQKA";

var seconds = "CDHS";

var n: dynamic;

var m: dynamic;

var a = cpp_array(55, 55);

var cur: dynamic;

var ans: dynamic;

var wh: dynamic;

func go(x: dynamic, y: dynamic)
{
  if ((y == m))
  {
    y = 0;
    x += 1;
  }
  if ((x == n))
  {
    var goods: dynamic;
    {
      var i1 = 0;
      while (((i1 + 2) < n))
      {
        {
          var j1 = 0;
          while (((j1 + 2) < m))
          {
            var f: dynamic;
            var s: dynamic;
            {
              var x = 0;
              while ((x < 3))
              {
                {
                  var y = 0;
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
    for (var c1 in goods)
    {
      for (var c2 in goods)
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
  var th = a[x][y];
  if (((th != "J1") && (th != "J2")))
  {
    return go(x, (y + 1));
  }
  for (var f in firsts)
  {
    for (var s in seconds)
    {
      var rep = [f, s];
      var ok = true;
      {
        var i = 0;
        while ((i < n))
        {
          {
            var j = 0;
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

func main()
{
  read(n, m);
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
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
      var x = ans[0].second;
      var y = ans[1].second;
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
