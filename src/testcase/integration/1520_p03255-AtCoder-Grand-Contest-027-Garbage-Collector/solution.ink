// Translated from solution.cpp.

var INF: dynamic = 1e16;

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var N: dynamic = cpp_uninitialized();
  var X: dynamic = cpp_uninitialized();
  read(N, X);
  var sum: dynamic = cpp_construct((N + 1), 0);
  {
    var i: dynamic = N;
    while ((i > 0))
    {
      read(sum[i]);
      i -= 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      sum[(i + 1)] += sum[i];
      i += 1;
    }
  }
  var ans: dynamic = INF;
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      var cnt: dynamic = (((N + i)) * X);
      var j: dynamic = i;
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
