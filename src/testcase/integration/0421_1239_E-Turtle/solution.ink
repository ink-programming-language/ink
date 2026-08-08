// Translated from solution.cpp.

func poww(a: dynamic, b: dynamic, md: dynamic)
{
  return (if ((!b)) 1 else (if ((b & 1)) ((a * poww(((a * a) % md), (b / 2), md)) % md) else (poww(((a * a) % md), (b / 2), md) % md)));
}

var maxn = 27;

var mxa = (50000 + 5);

var inf = 9223372036854775807;

var mod = (1e9 + 7);

var n: dynamic;

var a = cpp_array((maxn * 2));

var ans: dynamic;

var s: dynamic;

var cnt = cpp_array(mxa);

var dp = cpp_array((maxn * mxa), maxn);

var v: dynamic;

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(n);
  {
    var i = 1;
    while ((i <= (2 * n)))
    {
      read(a[i]);
      s += a[i];
      cnt[a[i]] += 1;
      i += 1;
    }
  }
  sort((a + 1), ((a + (2 * n)) + 1));
  s -= ((a[1] + a[2]));
  dp[0][0] = [1, 0];
  {
    var i = 3;
    while ((i <= (2 * n)))
    {
      {
        var j = (n - 1);
        while ((j >= 1))
        {
          {
            var k = s;
            while ((k >= a[i]))
            {
              if ((dp[(j - 1)][(k - a[i])].first && (!dp[j][k].first)))
              {
                dp[j][k] = [1, a[i]];
              }
              k -= 1;
            }
          }
          j -= 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < (maxn * mxa)))
    {
      if (((dp[(n - 1)][i].first != 0) && (i >= (s - i))))
      {
        ans = i;
        break;
      }
      i += 1;
    }
  }
  v.push_back(a[1]);
  cnt[a[1]] -= 1;
  var cur = (n - 1);
  while (cur)
  {
    v.push_back(dp[cur][ans].second);
    cnt[dp[cur][ans].second] -= 1;
    ans = (ans - dp[cur][ans].second);
    cur -= 1;
  }
  sort((v).begin(), (v).end());
  for (var u in v)
  {
    write(u, " ");
  }
  write("\n");
  {
    var i = (mxa - 1);
    while ((i >= 0))
    {
      while (cpp_update(cnt[i], "--"))
      {
        write(i, " ");
      }
      i -= 1;
    }
  }
}
