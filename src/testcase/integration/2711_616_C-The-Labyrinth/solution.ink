// Translated from solution.cpp.

var MN = 2000111;

var lab = cpp_array(MN);

class DSU
{
  func init(n: dynamic)
  {
      {
        var i = 0;
        var a = ((n + 1));
        while ((i < a))
        {
          lab[i] = -1;
          i += 1;
        }
      }
    }
  func getRoot(u: dynamic)
  {
      if ((lab[u] < 0))
      {
        return u;
      }
      return cpp_assign(lab[u], "=", getRoot(lab[u]));
    }
  func merge(u: dynamic, v: dynamic)
  {
      u = getRoot(u);
      v = getRoot(v);
      if ((u == v))
      {
        return false;
      }
      if ((lab[u] > lab[v]))
      {
        swap(u, v);
      }
      lab[u] += lab[v];
      lab[v] = u;
      return true;
    }
}

var m: dynamic;

var n: dynamic;

var a = cpp_array(1011, 1011);

var di = [-1, 1, 0, 0];

var dj = [0, 0, -1, 1];

func outside(i: dynamic, j: dynamic)
{
  return ((((i < 1) || (i > m)) || (j < 1)) || (j > n));
}

func id(i: dynamic, j: dynamic)
{
  return ((((i - 1)) * n) + j);
}

func main()
{
  while ((scanf("%d%d\n", (&m), (&n)) == 2))
  {
    {
      var i = (1);
      var b = (m);
      while ((i <= b))
      {
        scanf("%s\n", (&a[i][1]));
        i += 1;
      }
    }
    var dsu: dynamic;
    dsu.init((((m + 1)) * ((n + 1))));
    {
      var i = (1);
      var b = (m);
      while ((i <= b))
      {
        {
          var j = (1);
          var b = (n);
          while ((j <= b))
          {
            {
              var dir = 0;
              var a = (4);
              while ((dir < a))
              {
                var ii = (i + di[dir]);
                var jj = (j + dj[dir]);
                if (outside(ii, jj))
                {
                  dir += 1;
                  continue;
                }
                if (((a[i][j] == cpp_char("*")) || (a[ii][jj] == cpp_char("*"))))
                {
                  dir += 1;
                  continue;
                }
                dsu.merge(id(i, j), id(ii, jj));
                dir += 1;
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var i = (1);
      var b = (m);
      while ((i <= b))
      {
        {
          var j = (1);
          var b = (n);
          while ((j <= b))
          {
            if ((a[i][j] == cpp_char(".")))
            {
              putchar(cpp_char("."));
            } else
            {
              var res = 1;
              var all: dynamic;
              {
                var dir = 0;
                var a = (4);
                while ((dir < a))
                {
                  var ii = (i + di[dir]);
                  var jj = (j + dj[dir]);
                  if (outside(ii, jj))
                  {
                    dir += 1;
                    continue;
                  }
                  if ((a[ii][jj] == cpp_char("*")))
                  {
                    dir += 1;
                    continue;
                  }
                  var t = dsu.getRoot(id(ii, jj));
                  if (all.count(t))
                  {
                    dir += 1;
                    continue;
                  }
                  all.insert(t);
                  res += (-lab[dsu.getRoot(id(ii, jj))]);
                  dir += 1;
                }
              }
              putchar(cpp_cast(((cpp_char("0") + (res % 10)))));
            }
            j += 1;
          }
        }
        puts("");
        i += 1;
      }
    }
  }
}
