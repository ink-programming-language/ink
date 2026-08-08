// Translated from solution.cpp.

var NMAX = 333333;

var WMAX = 111111;

var dp = cpp_array(NMAX);

var n: dynamic;

var m: dynamic;

var e = cpp_array(WMAX);

func main()
{
  cin.sync_with_stdio(0);
  read(n, m);
  {
    var i = 0;
    while ((i < m))
    {
      var u: dynamic;
      var v: dynamic;
      var w: dynamic;
      read(u, v, w);
      e[w].push_back(pair(u, v));
      i += 1;
    }
  }
  {
    var w = 1;
    while ((w < WMAX))
    {
      var query: dynamic;
      {
        var j = 0;
        while ((j < e[w].size()))
        {
          var road = e[w][j];
          query.push_back(pair(road.second, (dp[road.first] + 1)));
          j += 1;
        }
      }
      {
        var i = 0;
        while ((i < query.size()))
        {
          dp[query[i].first] = max(dp[query[i].first], query[i].second);
          i += 1;
        }
      }
      w += 1;
    }
  }
  write((*max_element((dp + 1), ((dp + 1) + n))));
}
