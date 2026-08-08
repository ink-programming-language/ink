// Translated from solution.cpp.

var INF = 1e9;

func main()
{
  var V: dynamic;
  var E: dynamic;
  read(V, E);
  {
    var i = 0;
    while ((i < E))
    {
      var s: dynamic;
      var t: dynamic;
      var d: dynamic;
      read(s, t, d);
      dist[s][t] = d;
      i += 1;
    }
  }
  var dp = cpp_construct(V, vector((1 << V), INF));
  dp[0][0] = 0;
  {
    var i = 0;
    while ((i < ((1 << V))))
    {
      {
        var j = 0;
        while ((j < V))
        {
          if ((!((i & ((1 << j))))))
          {
            {
              var k = 0;
              while ((k < V))
              {
                dp[j][(i + ((1 << j)))] = min(dp[j][(i + ((1 << j)))], (dp[k][i] + dist[k][j]));
                k += 1;
              }
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans = dp[0][(((1 << V)) - 1)];
  if ((ans == INF))
  {
    write(-1, "\n");
  } else
  {
    write(ans, "\n");
  }
}
