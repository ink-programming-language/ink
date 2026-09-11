// Translated from solution.cpp.

func fast_power(val: dynamic, deg: dynamic, mod: dynamic = 1000000007) -> dynamic
{
  if ((!deg))
  {
    return (1 % mod);
  }
  if ((deg & 1))
  {
    return ((fast_power(val, (deg - 1), mod) * val) % mod);
  }
  var res: dynamic = fast_power(val, (deg >> 1), mod);
  return (((res * res)) % mod);
}

func MMI(a: dynamic, mm: dynamic = 1000000007) -> dynamic
{
  return (fast_power((a % mm), (mm - 2), mm) % mm);
}

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_array(5011);

var v: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

func check() -> dynamic
{
  var i: dynamic = 0;
  while ((m[i] == 1))
  {
    i += 1;
  }
  var end: dynamic = v[i].second.first;
  i += 1;
  {
    while ((i < n))
    {
      if ((!m[i]))
      {
        if ((end > v[i].first))
        {
          return false;
        }
        end = v[i].second.first;
      }
      i += 1;
    }
  }
  return true;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      read(a, b);
      v.push_back(make_pair(a, make_pair(b, (i + 1))));
      i += 1;
    }
  }
  sort(v.begin(), v.end());
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      m[i] = 1;
      if (check())
      {
        ans.push_back(v[i].second.second);
      }
      m[i] = 0;
      i += 1;
    }
  }
  sort(ans.begin(), ans.end());
  write(cpp_cast(ans.size()), "\n");
  for (var i: dynamic in ans)
  {
    write(i, " ");
  }
  return 0;
}
