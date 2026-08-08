// Translated from solution.cpp.

var INF = 1e16;

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var N: dynamic;
  var X: dynamic;
  read(N, X);
  var sum = cpp_construct((N + 1), 0);
  {
    var i = N;
    while ((i > 0))
    {
      read(sum[i]);
      i -= 1;
    }
  }
  {
    var i = 0;
    while ((i < N))
    {
      sum[(i + 1)] += sum[i];
      i += 1;
    }
  }
  var ans = INF;
  {
    var i = 1;
    while ((i <= N))
    {
      var cnt = (((N + i)) * X);
      var j = i;
      {
        while ((j <= N))
        {
          cnt += (((sum[j] - sum[(j - i)])) * max(5, ((2 * (((j / i) - 1))) + 3)));
          if ((ans < cnt))
          {
            break;
          }
          j += i;
        }
      }
      cnt += (((sum[N] - sum[(j - i)])) * max(5, ((2 * (((j / i) - 1))) + 3)));
      ans = min(ans, cnt);
      i += 1;
    }
  }
  write(ans, cpp_char("\n"));
}
