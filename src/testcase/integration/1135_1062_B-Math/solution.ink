// Translated from solution.cpp.

var dx: dynamic = [0, 0, 1, -1, 1, 1, -1, -1];

var dy: dynamic = [1, -1, 0, 0, -1, 1, 1, -1];

var mod: dynamic = (1e9 + 7);

func dcmp(x: dynamic, y: dynamic) -> dynamic
{
  return  ((fabs((x - y)) <= 1e-12)) ? 0 :  ((x < y)) ? -1 : 1;
}

func fast() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(null);
  cout.tie(null);
}

var n: dynamic = cpp_uninitialized();

var mp: dynamic = cpp_uninitialized();

func pf() -> dynamic
{
  {
    var i: dynamic = 2;
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

func main() -> dynamic
{
  fast();
  read(n);
  pf();
  var cnt: dynamic = 0;
  var mx: dynamic = 0;
  var num: dynamic = 1;
  for (var e: dynamic in mp)
  {
    mx = max(e.second, mx);
    num *= e.first;
  }
  var lo: dynamic = 0;
  var hi: dynamic = 30;
  var ans: dynamic = 1;
  while ((lo <= hi))
  {
    var md: dynamic = ((lo + (((hi - lo)) / 2)));
    if ((((1 << md)) >= mx))
    {
      hi = (md - 1);
      ans = md;
    } else
    {
      lo = (md + 1);
    }
  }
  for (var e: dynamic in mp)
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
