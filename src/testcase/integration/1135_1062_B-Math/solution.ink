// Translated from solution.cpp.

var dx = [0, 0, 1, -1, 1, 1, -1, -1];

var dy = [1, -1, 0, 0, -1, 1, 1, -1];

var mod = (1e9 + 7);

func dcmp(x: dynamic, y: dynamic)
{
  return if ((fabs((x - y)) <= 1e-12)) 0 else if ((x < y)) -1 else 1;
}

func fast()
{
  ios_base.sync_with_stdio(0);
  cin.tie(null);
  cout.tie(null);
}

var n: dynamic;

var mp: dynamic;

func pf()
{
  {
    var i = 2;
    while ((((1 * i) * i) <= n))
    {
      while (((n % i) == 0))
      {
        mp[i] += 1;
        n /= i;
      }
      i += 1;
    }
  }
  if ((n > 1))
  {
    mp[n] += 1;
  }
}

func main()
{
  fast();
  read(n);
  pf();
  var cnt = 0;
  var mx = 0;
  var num = 1;
  for (var e in mp)
  {
    mx = max(e.second, mx);
    num *= e.first;
  }
  var lo = 0;
  var hi = 30;
  var ans = 1;
  while ((lo <= hi))
  {
    var md = ((lo + (((hi - lo)) / 2)));
    if ((((1 << md)) >= mx))
    {
      hi = (md - 1);
      ans = md;
    } else
    {
      lo = (md + 1);
    }
  }
  for (var e in mp)
  {
    if ((e.second < ((1 << ans))))
    {
      cnt += 1;
      break;
    }
  }
  write(num, " ");
  write((cnt + ans), cpp_char("\n"));
  return 0;
}
