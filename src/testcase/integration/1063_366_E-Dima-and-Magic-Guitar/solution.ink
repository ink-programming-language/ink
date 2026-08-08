// Translated from solution.cpp.

func dist(a: dynamic, b: dynamic)
{
  return (abs((a.first - b.first)) + abs((a.second - b.second)));
}

var N = 2000;

var K = 10;

var vis = cpp_array(K);

var sla = cpp_array(4, K);

var tans = vector(K, vector(K));

func solvetask()
{
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  var s: dynamic;
  read(n, m, k, s);
  var aux = [[0, 0], [(n - 1), 0], [(n - 1), (m - 1)], [0, (m - 1)]];
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < m))
        {
          var num: dynamic;
          read(num);
          if ((!vis[num]))
          {
            {
              var y = 0;
              while ((y < 4))
              {
                sla[num][y] = [i, j];
                y += 1;
              }
            }
            vis[num] = 1;
            j += 1;
            continue;
          }
          {
            var y = 0;
            while ((y < 4))
            {
              if ((dist(sla[num][y], aux[y]) > dist([i, j], aux[y])))
              {
                sla[num][y] = [i, j];
              }
              y += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var n1 = 1;
    while ((n1 <= k))
    {
      {
        var n2 = n1;
        while ((n2 <= k))
        {
          if (((!vis[n1]) || (!vis[n2])))
          {
            n2 += 1;
            continue;
          }
          {
            var i = 0;
            while ((i < 4))
            {
              {
                var j = 0;
                while ((j < 4))
                {
                  tans[n1][n2] = max(tans[n1][n2], dist(sla[n1][i], sla[n2][j]));
                  tans[n2][n1] = max(tans[n2][n1], tans[n1][n2]);
                  j += 1;
                }
              }
              i += 1;
            }
          }
          n2 += 1;
        }
      }
      n1 += 1;
    }
  }
  var ans = 0;
  var ant: dynamic;
  var cur: dynamic;
  read(ant);
  {
    var y = 1;
    while ((y < s))
    {
      read(cur);
      ans = max(ans, tans[ant][cur]);
      ant = cur;
      y += 1;
    }
  }
  write(ans, cpp_char("\n"));
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var t = 1;
  while (cpp_update(t, "--"))
  {
    solvetask();
  }
}
