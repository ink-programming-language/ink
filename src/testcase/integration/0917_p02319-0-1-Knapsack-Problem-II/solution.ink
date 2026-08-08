// Translated from solution.cpp.

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var N: dynamic;
  var W: dynamic;
  read(N, W);
  {
    var i = 0;
    while ((i < N))
    {
      read(v.at(i), w.at(i));
      i += 1;
    }
  }
  var dp = cpp_construct((N + 1), vector(10001, INT_MAX));
  dp.at(0).at(0) = 0;
  {
    var i = 0;
    while ((i < N))
    {
      {
        var j = 0;
        while ((j <= 10000))
        {
          if ((j >= v.at(i)))
          {
            dp.at((i + 1)).at(j) = min(dp.at(i).at(j), (dp.at(i).at((j - v.at(i))) + w.at(i)));
          } else
          {
            dp.at((i + 1)).at(j) = dp.at(i).at(j);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans = 0;
  {
    var j = 0;
    while ((j <= 10000))
    {
      if ((dp.at(N).at(j) <= W))
      {
        ans = j;
      }
      j += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
