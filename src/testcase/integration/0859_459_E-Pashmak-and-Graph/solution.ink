// Translated from solution.cpp.

var NMAX: dynamic = 333333;

var WMAX: dynamic = 111111;

var dp: dynamic = cpp_array(NMAX);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var e: dynamic = cpp_array(WMAX);

func main() -> dynamic
{
  cin.sync_with_stdio(0);
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      var w: dynamic = cpp_uninitialized();
      read(u, v, w);
      e[w].push_back(pair(u, v));
      i += 1;
    }
  }
  {
    var w: dynamic = 1;
    while ((w < WMAX))
    {
      var query: dynamic = cpp_uninitialized();
      {
        var j: dynamic = 0;
        while ((j < e[w].size()))
        {
          var road: dynamic = e[w][j];
          query.push_back(pair(road.second, (dp[road.first] + 1)));
          j += 1;
        }
      }
      {
        var i: dynamic = 0;
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
