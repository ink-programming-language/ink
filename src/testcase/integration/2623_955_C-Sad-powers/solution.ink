// Translated from solution.cpp.

var OO: dynamic = 0x3f3f3f3f;

var MOD: dynamic = (1e9 + 7);

var pi: dynamic = acos(-1);

var EPS: dynamic = 1e-9;

var MAX: dynamic = 1e6;

var l: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

var v: dynamic = cpp_uninitialized();

var vec: dynamic = cpp_uninitialized();

var st: dynamic = cpp_uninitialized();

func FindRoot(x: dynamic) -> dynamic
{
  var lo: dynamic = 0;
  var hi: dynamic = 1e10;
  var mid: dynamic = cpp_uninitialized();
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

func initial() -> dynamic
{
  v.push_back(1);
  {
    var i: dynamic = 2;
    while ((i <= 1e6))
    {
      {
        var j: dynamic = ((i * i) * i);
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
  for (var it: dynamic in st)
  {
    v.push_back(it);
  }
  for (var it: dynamic in v)
  {
    var sq: dynamic = FindRoot(it);
    if (((sq * sq) == it))
    {
      continue;
    }
    vec.push_back(it);
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  initial();
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    read(l, r);
    var it1: dynamic = (upper_bound(vec.begin(), vec.end(), r) - v.begin());
    var it2: dynamic = (lower_bound(vec.begin(), vec.end(), l) - v.begin());
    var ans: dynamic = (it1 - it2);
    write(((ans + cpp_cast(FindRoot(r))) - cpp_cast(FindRoot((l - 1)))), "\n");
  }
}
