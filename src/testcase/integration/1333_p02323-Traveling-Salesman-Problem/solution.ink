// Translated from solution.cpp.

var INF: dynamic = 1e9;

func main() -> dynamic
{
  var V: dynamic = cpp_uninitialized();
  var E: dynamic = cpp_uninitialized();
  read(V, E);
  {
    var i: dynamic = 0;
    while ((i < E))
    {
      var s: dynamic = cpp_uninitialized();
      var t: dynamic = cpp_uninitialized();
      var d: dynamic = cpp_uninitialized();
      read(s, t, d);
      dist[s][t] = d;
      i += 1;
    }
  }
  var dp: dynamic = cpp_construct(V, vector((1 << V), INF));
  dp[0][0] = 0;
  {
    var i: dynamic = 0;
    while ((i < ((1 << V))))
    {
      {
        var j: dynamic = 0;
        while ((j < V))
        {
          if ((!((i & ((1 << j))))))
          {
            {
              var k: dynamic = 0;
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
  var ans: dynamic = dp[0][(((1 << V)) - 1)];
  if ((ans == INF))
  {
    write(-1, "\n");
  } else
  {
    write(ans, "\n");
  }
}
