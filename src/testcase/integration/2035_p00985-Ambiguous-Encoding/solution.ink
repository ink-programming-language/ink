// Translated from solution.cpp.

var INF = (((1 << 30)) - 1);

func chmin(a: dynamic, b: dynamic)
{
  a = min(a, b);
}

func main()
{
  var N: dynamic;
  var dp = cpp_array(20, 1005);
  var S = cpp_array(1005);
  var pq: dynamic;
  fill_n((*dp), (1005 * 20), INF);
  read(N);
  {
    var i = 0;
    while ((i < N))
    {
      read(S[i]);
      dp[i][0] = 0;
      pq.emplace(0, Pi(i, 0));
      i += 1;
    }
  }
  while ((!pq.empty()))
  {
    var dat = pq.top();
    pq.pop();
    var cost = dat.first;
    var node = dat.second.first;
    var pos = dat.second.second;
    var nodesize = S[node].size();
    var remainsize = (nodesize - pos);
    {
      var i = 0;
      while ((i < N))
      {
        if (((i == node) && (pos == 0)))
        {
          i += 1;
          continue;
        }
        var len = S[i].size();
        if ((len > remainsize))
        {
          if ((S[node].substr(pos, remainsize) == S[i].substr(0, remainsize)))
          {
            var ncost = (cost + remainsize);
            if ((dp[i][remainsize] > ncost))
            {
              chmin(dp[i][remainsize], ncost);
              pq.emplace(ncost, Pi(i, remainsize));
            }
          }
        } else if ((S[node].substr(pos, len) == S[i]))
        {
          var ncost = (cost + len);
          if ((dp[node][(pos + len)] > ncost))
          {
            chmin(dp[node][(pos + len)], ncost);
            if ((len < remainsize))
            {
              pq.emplace(ncost, Pi(node, (pos + len)));
            }
          }
        }
        i += 1;
      }
    }
  }
  var minv = INF;
  {
    var i = 0;
    while ((i < N))
    {
      minv = min(minv, dp[i][S[i].size()]);
      i += 1;
    }
  }
  if ((minv == INF))
  {
    write(0, "\n");
  } else
  {
    write(minv, "\n");
  }
  return (0);
}
