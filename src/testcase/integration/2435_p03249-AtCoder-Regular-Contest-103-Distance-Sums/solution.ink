// Translated from solution.cpp.

var MX = (1e5 + 5);

var a = cpp_array(MX);

var mp: dynamic;

var n: dynamic;

var p = cpp_array(MX);

var visit = cpp_array(MX);

var s = cpp_array(MX);

var dp = cpp_array(MX);

func main()
{
  cin.tie(0);
  cout.tie(0);
  ios_base.sync_with_stdio(0);
  read(n);
  var i: dynamic;
  {
    i = 1;
    while ((i <= n))
    {
      read(a[i].first);
      a[i].second = i;
      mp[a[i].first] = i;
      i += 1;
    }
  }
  sort((a + 1), ((a + n) + 1));
  {
    i = n;
    while ((i >= 2))
    {
      s[a[i].second] += 1;
      var k = mp[((a[i].first + (2 * s[a[i].second])) - n)];
      if ((((k == 0) || (k == a[i].second)) || (((a[i].first + (2 * s[a[i].second])) - n) >= a[i].first)))
      {
        write(-1);
        return 0;
      }
      s[k] += s[a[i].second];
      dp[k] += (dp[a[i].second] + s[a[i].second]);
      p[a[i].second] = k;
      i -= 1;
    }
  }
  if ((dp[a[1].second] != a[1].first))
  {
    write(-1);
    return 0;
  }
  {
    i = 2;
    while ((i <= n))
    {
      write(a[i].second, " ", p[a[i].second], "\n");
      i += 1;
    }
  }
  return 0;
}
