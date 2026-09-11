// Translated from solution.cpp.

func poww(a: dynamic, b: dynamic, md: dynamic) -> dynamic
{
  return ( ((!b)) ? 1 : ( ((b & 1)) ? ((a * poww(((a * a) % md), (b / 2), md)) % md) : (poww(((a * a) % md), (b / 2), md) % md)));
}

var maxn: dynamic = 27;

var mxa: dynamic = (50000 + 5);

var inf: dynamic = 9223372036854775807;

var mod: dynamic = (1e9 + 7);

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array((maxn * 2));

var ans: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_array(mxa);

var dp: dynamic = cpp_array((maxn * mxa), maxn);

var v: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(n);
  {
    var i: dynamic = 1;
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
    var i: dynamic = 3;
    while ((i <= (2 * n)))
    {
      {
        var j: dynamic = (n - 1);
        while ((j >= 1))
        {
          {
            var k: dynamic = s;
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
    var i: dynamic = 0;
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
  var cur: dynamic = (n - 1);
  while (cur)
  {
    v.push_back(dp[cur][ans].second);
    cnt[dp[cur][ans].second] -= 1;
    ans = (ans - dp[cur][ans].second);
    cur -= 1;
  }
  sort((v).begin(), (v).end());
  for (var u: dynamic in v)
  {
    write(u, " ");
  }
  write("\n");
  {
    var i: dynamic = (mxa - 1);
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
