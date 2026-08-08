// Translated from solution.cpp.

var OO = 0x3f3f3f3f;

var MOD = (1e9 + 7);

var pi = acos(-1);

var EPS = 1e-9;

var MAX = 1e6;

var l: dynamic;

var r: dynamic;

var v: dynamic;

var vec: dynamic;

var st: dynamic;

func FindRoot(x: dynamic)
{
  var lo = 0;
  var hi = 1e10;
  var mid: dynamic;
  while ((lo < (hi - 1)))
  {
    mid = ((lo + hi) >> 1);
    if (((mid * mid) > x))
    {
      hi = mid;
    } else
    {
      lo = mid;
    }
  }
  return lo;
}

func initial()
{
  v.push_back(1);
  {
    var i = 2;
    while ((i <= 1e6))
    {
      {
        var j = ((i * i) * i);
        while ((j <= 1e18))
        {
          st.insert(j);
          if ((j > (1e18 / i)))
          {
            break;
          }
          j *= i;
        }
      }
      i += 1;
    }
  }
  for (var it in st)
  {
    v.push_back(it);
  }
  for (var it in v)
  {
    var sq = FindRoot(it);
    if (((sq * sq) == it))
    {
      continue;
    }
    vec.push_back(it);
  }
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  initial();
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    read(l, r);
    var it1 = (upper_bound(vec.begin(), vec.end(), r) - v.begin());
    var it2 = (lower_bound(vec.begin(), vec.end(), l) - v.begin());
    var ans = (it1 - it2);
    write(((ans + cpp_cast(FindRoot(r))) - cpp_cast(FindRoot((l - 1)))), "\n");
  }
}
