// Translated from solution.cpp.

func dist(a: dynamic, b: dynamic) -> dynamic
{
  return (abs((a.first - b.first)) + abs((a.second - b.second)));
}

var N: dynamic = 2000;

var K: dynamic = 10;

var vis: dynamic = cpp_array(K);

var sla: dynamic = cpp_array(4, K);

var tans: dynamic = vector(K, vector(K));

func solvetask() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  read(n, m, k, s);
  var aux: dynamic = [[0, 0], [(n - 1), 0], [(n - 1), (m - 1)], [0, (m - 1)]];
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          var num: dynamic = cpp_uninitialized();
          read(num);
          if ((!vis[num]))
          {
            {
              var y: dynamic = 0;
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
            var y: dynamic = 0;
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
    var n1: dynamic = 1;
    while ((n1 <= k))
    {
      {
        var n2: dynamic = n1;
        while ((n2 <= k))
        {
          if (((!vis[n1]) || (!vis[n2])))
          {
            n2 += 1;
            continue;
          }
          {
            var i: dynamic = 0;
            while ((i < 4))
            {
              {
                var j: dynamic = 0;
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
  var ans: dynamic = 0;
  var ant: dynamic = cpp_uninitialized();
  var cur: dynamic = cpp_uninitialized();
  read(ant);
  {
    var y: dynamic = 1;
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var t: dynamic = 1;
  while (cpp_update(t, "--"))
  {
    solvetask();
  }
}
