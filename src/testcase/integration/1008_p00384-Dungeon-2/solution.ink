// Translated from solution.cpp.

var eps = 1e-9;

var answer = -1e9;

func dfs(edges: dynamic, scores: dynamic, now: dynamic, from_cpp: dynamic)
{
  var ans = cpp_construct(3, -1e9);
  ans[0] = scores[now];
  for (var e in edges[now])
  {
    if ((e != from_cpp))
    {
      var v = dfs(edges, scores, e, now);
      {
        var l = 0;
        while ((l < 3))
        {
          {
            var r = 0;
            while ((r < 3))
            {
              if (((l + r) <= 2))
              {
                if ((r == 0))
                {
                  nextans[(l + r)] = max(nextans[(l + r)], ((ans[l] + v[r]) - 2));
                  if ((((l + r) + 1) <= 2))
                  {
                    nextans[((l + r) + 1)] = max(nextans[((l + r) + 1)], ((ans[l] + v[r]) - 1));
                  }
                } else if ((r == 1))
                {
                  nextans[(l + r)] = max(nextans[(l + r)], ((ans[l] + v[r]) - 1));
                } else
                {
                  nextans[(l + r)] = max(nextans[(l + r)], ((ans[l] + v[r]) - 2));
                }
              }
              r += 1;
            }
          }
          l += 1;
        }
      }
      ans = nextans;
    }
  }
  for (var a in ans)
  {
    answer = max(answer, a);
  }
  return ans;
}

func main()
{
  var N: dynamic;
  read(N);
  {
    var i = 0;
    while ((i < N))
    {
      var a: dynamic;
      read(a);
      scores[i] = a;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < (N - 1)))
    {
      var a: dynamic;
      var b: dynamic;
      read(a, b);
      a -= 1;
      b -= 1;
      edges[a].push_back(b);
      edges[b].push_back(a);
      i += 1;
    }
  }
  var v = dfs(edges, scores, 0, -1);
  write(answer, "\n");
  return 0;
}
